#include "shadow.hpp"

#include "../app/application.hpp"
#include "../ecs/scene.hpp"
#include "../graphics/layout.hpp"
#include "nodes/models.hpp"
#include "nodes/lights.hpp"

namespace Physbuzz {

namespace Builtin {

bool RenderPipelineShadowDirectional::build(const glm::uvec2 &resolution) {
    if (ResourceRegistry<RenderPipeline>::contains(Resource)) {
        return true;
    }

    bool success = true;

    if (!ResourceRegistry<DynamicBuffer>::contains(ResourceModel)) {
        success &= ResourceRegistry<DynamicBuffer>::insert(
            ResourceModel,
            {{
                .type = DynamicBuffer::Type::Structured,
            }},
            sizeof(RenderNodeModels::ModelBuffer));
    }

    if (!ResourceRegistry<Attachment>::contains(ResourceAttachment)) {
        success &= ResourceRegistry<Attachment>::insert(
            ResourceAttachment,
            {{
                .type = Attachment::Type::Dim2D,
                .sampler = {{
                    .type = Sampler::Type::Linear,
                }},
                .usage = Attachment::Usage::Depth,
                .format = Attachment::Format::eD32Sfloat,
            }},
            resolution);
    }

    if (!ResourceRegistry<PipelineLayout>::contains(ResourceLayoutFrame)) {
        success &= ResourceRegistry<PipelineLayout>::insert(
            ResourceLayoutFrame,
            {{
                .bindings = {
                    {
                        // shadow
                        .type = PipelineLayout::Type::eStorageBuffer,
                    },
                    {
                        // instance
                        .type = PipelineLayout::Type::eStorageBuffer,
                    },
                },
            }});
    }

    success &= ResourceRegistry<RenderPipeline>::insert(
        Resource,
        {{
            .module = "builtin/depth/Dim2D",
            .description = &Model::Vertex::Description,
            .rasterization = {
                .cullMode = vk::CullModeFlagBits::eFront,
            },
            .blend = {
                .attachments = {{}},
            },
            .formats = {
                .color = {},
                .depth = RenderPipeline::Format::eD32Sfloat,
            },
            .layouts = {
                .resources = {
                    ResourceLayoutFrame,
                },
            },
            .attachments = {
                .colors = {},
                // .depth = {0},
            },
        }});

    return success;
}

bool RenderPipelineShadowPoint::build() {
    if (ResourceRegistry<RenderPipeline>::contains(Resource)) {
        return true;
    }

    // return ResourceRegistry<ShaderPipeline>::insert(
    //     Resource,
    //     {{
    //         .draw = [](const ShaderPipeline *resource, Scene &scene, ObjectID object) {
    //             const auto [render] = scene.getComponent<RenderComponent>(object);
    //
    //             resource->setUniform("PBZ_Model", render.transform.matrix);
    //
    //             for (const auto &[mesh, _] : render.model->getMeshs()) {
    //                 mesh.draw();
    //             }
    //         },
    //     }});

    return true;
}

} // namespace Builtin

ShadowRenderer::ShadowRenderer(const Info &info)
    : m_Info(info) {}

bool ShadowRenderer::build() {
    bool success = true;

    success &= Builtin::RenderPipelineShadowDirectional::build(m_Info.resolution);
    success &= Builtin::RenderPipelineShadowPoint::build();

    m_Graph.add("builtin/shadow/models", Builtin::RenderNodeModels::build(Builtin::RenderPipelineShadowDirectional::ResourceModel, m_Objects, m_Batches));
    m_Graph.add("builtin/lights", Builtin::RenderNodeLights::build());

    m_Graph.add(
        Output,
        {
            .description = {
                .buffers = {
                    .input = {
                        {
                            Builtin::RenderPipelineShadowDirectional::ResourceModel,
                            {
                                .stage = RenderNode::Stage::Vertex,
                            },
                        },
                        {
                            Builtin::RenderNodeLights::ResourceBufferDirectional,
                            {
                                .stage = RenderNode::Stage::Fragment,
                            },
                        },
                    },
                },
                .attachments = {
                    .output = {
                        {
                            Builtin::RenderPipelineShadowDirectional::ResourceAttachment,
                            {
                                .stage = RenderNode::Stage::Fragment,
                            },
                        },
                    },
                },
            },
            .execute = [this](Scene *scene, const RenderContext &context) {
                vk::RenderingAttachmentInfo depthAttachment = {
                    .imageView = Builtin::RenderPipelineShadowDirectional::ResourceAttachment->getRingData()[context.frameInFlight].view,
                    .imageLayout = vk::ImageLayout::eDepthStencilAttachmentOptimal,
                    .loadOp = vk::AttachmentLoadOp::eClear,
                    .storeOp = vk::AttachmentStoreOp::eStore,
                    .clearValue = vk::ClearDepthStencilValue{1.0f, 0},
                };

                context.command.setViewport(0, vk::Viewport{0.0f, 0.0f, static_cast<float>(m_Info.resolution.x), static_cast<float>(m_Info.resolution.y), 0.0f, 1.0f});
                context.command.setScissor(0, vk::Rect2D{{0, 0}, {m_Info.resolution.x, m_Info.resolution.y}});

                // issue draw calls
                context.command.beginRendering({
                    .renderArea = {
                        .offset = {0, 0},
                        .extent = {m_Info.resolution.x, m_Info.resolution.y},
                    },
                    .layerCount = 1,
                    .viewMask = {},
                    .pDepthAttachment = &depthAttachment,
                    .pStencilAttachment = {},
                });

                // bind resources
                Builtin::RenderPipelineShadowDirectional::Resource->bind(context);
                App::LayoutAllocator.bind(context, Builtin::RenderPipelineShadowDirectional::Resource);

                std::uint32_t object = 0;
                for (const auto &[mesh, batch] : m_Batches) {
                    if (mesh->getDescription() != Builtin::RenderPipelineShadowDirectional::Resource->getInfo().description) {
                        Logger::ERROR("[ShadowRenderer] Incompatible vertex state descriptions.");
                        continue;
                    }

                    mesh->draw(context, batch, object);
                    object += batch;
                }

                context.command.endRendering();
            },
        });

    if (success) {
        App::LayoutAllocator.write(
            Builtin::RenderPipelineShadowDirectional::ResourceLayoutFrame,
            Builtin::RenderNodeLights::ResourceBufferDirectional,
            0);

        App::LayoutAllocator.write(
            Builtin::RenderPipelineShadowDirectional::ResourceLayoutFrame,
            Builtin::RenderPipelineShadowDirectional::ResourceModel,
            1);
    }

    return success;
}

bool ShadowRenderer::destroy() {
    return true;
}

void ShadowRenderer::tickPoint() const {
    // m_Framebuffers.point.clear();
    // m_Framebuffers.point.bind();
    //
    // glm::mat4 projection = glm::perspective(glm::radians(90.0f), 1.0f, 1.0f, depth);
    //
    // if (!Builtin::ShaderShaderDepthCubemap::Resource->reload()) {
    //     return;
    // }
    //
    // Builtin::ShaderShaderDepthCubemap::Resource->bind();
    // Builtin::ShaderShaderDepthCubemap::Resource->setUniform("PBZ_FarPlane", depth);
    //
    // for (const auto [_, light] : m_Scene->getComponents<PointLightComponent>()) {
    //     std::array matrices = {
    //         projection * glm::lookAt(light.position, light.position + glm::vec3(1.0f, 0.0f, 0.0f), {0.0f, -1.0f, 0.0f}),
    //         projection * glm::lookAt(light.position, light.position + glm::vec3(-1.0f, 0.0f, 0.0f), {0.0f, -1.0f, 0.0f}),
    //         projection * glm::lookAt(light.position, light.position + glm::vec3(0.0f, 1.0f, 0.0f), {0.0f, 0.0f, 1.0f}),
    //         projection * glm::lookAt(light.position, light.position + glm::vec3(0.0f, -1.0f, 0.0f), {0.0f, 0.0f, -1.0f}),
    //         projection * glm::lookAt(light.position, light.position + glm::vec3(0.0f, 0.0f, 1.0f), {0.0f, -1.0f, 0.0f}),
    //         projection * glm::lookAt(light.position, light.position + glm::vec3(0.0f, 0.0f, -1.0f), {0.0f, -1.0f, 0.0f}),
    //     };
    //
    //     Builtin::ShaderShaderDepthCubemap::Resource->setUniform("PBZ_LightPosition", light.position);
    //     for (std::size_t i = 0; i < matrices.size(); i++) {
    //         Builtin::ShaderShaderDepthCubemap::Resource->setUniform(std::format("PBZ_LightMatrix[{}]", i), matrices[i]);
    //     }
    //
    //     for (const auto &object : m_Objects) {
    //         Builtin::ShaderShaderDepthCubemap::Resource->draw(*m_Scene, object);
    //     }
    // }
    //
    // Builtin::ShaderShaderDepthCubemap::Resource->unbind();
    // m_Framebuffers.point.unbind();
}

void ShadowRenderer::resize(const glm::ivec2 &resolution) {
    // m_Framebuffers.directional.resize(resolution);
    // m_Framebuffers.point.resize(glm::ivec2(glm::max(resolution.x, resolution.y)));
}

const RenderGraph &ShadowRenderer::getGraph() const {
    return m_Graph;
}

const ShadowRenderer::Info &ShadowRenderer::getInfo() const {
    return m_Info;
}

} // namespace Physbuzz
