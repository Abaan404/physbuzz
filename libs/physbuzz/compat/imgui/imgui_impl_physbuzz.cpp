#include "imgui_impl_physbuzz.hpp"

#include <imgui_impl_glfw.h>
#include <imgui_impl_vulkan.h>

#include "../../app/application.hpp"
#include "../../app/deletion.hpp"
#include "../../debug/macros.hpp"
#include "../../ecs/scene.hpp"
#include "../../events/descriptor.hpp"
#include "../../graphics/descriptors/sampler.hpp"
#include "../../graphics/descriptors/texture.hpp"
#include "../../graphics/renderer.hpp"

namespace Physbuzz {

namespace Builtin {

bool RenderPipelineImGui::build() {
    bool success = true;

    if (!ResourceRegistry<Sampler>::contains(ResourceSampler)) {
        success &= ResourceRegistry<Sampler>::insert(
            ResourceSampler,
            {{
                .type = Physbuzz::Sampler::Type::Linear,
            }});
    }

    return success;
}

} // namespace Builtin

ImGuiRenderer::ImGuiRenderer(const Info &info)
    : m_Info(info) {}

bool ImGuiRenderer::build() {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    if (!Builtin::RenderPipelineImGui::build()) {
        Logger::ERROR("[ImGuiRenderer] Could not build ImGui resources.");
        return false;
    }

    if (!ImGui_ImplGlfw_InitForVulkan(*m_Info.window, true)) {
        Logger::ERROR("[ImGuiRenderer] Could not initialize ImGui with GLFW.");
        return false;
    }

    ImGui_ImplVulkan_InitInfo initInfo = {
        .ApiVersion = vk::ApiVersion14,
        .Instance = App::Instance,
        .PhysicalDevice = App::PhysicalDevice,
        .Device = App::Device,
        .QueueFamily = App::Indices.graphics,
        .Queue = App::Queues.graphics,
        .DescriptorPool = nullptr, // let imgui create a pool
        .DescriptorPoolSize = 1000,
        .MinImageCount = detail::MAX_FRAMES_IN_FLIGHT,
        .ImageCount = detail::MAX_FRAMES_IN_FLIGHT,
        .PipelineInfoMain = {
            .RenderPass = {},
            .Subpass = {},
            .MSAASamples = static_cast<VkSampleCountFlagBits>(vk::SampleCountFlagBits::e1),
            .PipelineRenderingCreateInfo = vk::PipelineRenderingCreateInfo{
                .colorAttachmentCount = 1,
                .pColorAttachmentFormats = &m_Info.window->getInfo().swapChain.format,
            },
        },
        .UseDynamicRendering = true,
        .CheckVkResultFn = [](VkResult err) {
            PBZ_VK_CHECK_RESULT(vk::Result(err));
        },
    };

    if (!ImGui_ImplVulkan_Init(&initInfo)) {
        Logger::ERROR("[ImGuiRenderer] Could not initialize ImGui with Vulkan.");
        return false;
    }

    m_Graph.add(
        Output,
        {
            .prepare = [](Scene *scene, const RenderContext &context) {
                // this is necessary for capturing current attachment in the ring buffer.
                scene->getSystem<ImGuiRenderer>()->m_FrameInFlight = context.frameInFlight;
            },
            .execute = [](Scene *, const RenderContext &context) {
                std::vector<vk::RenderingAttachmentInfo> colorAttachments = {
                    {
                        .imageView = context.color.view,
                        .imageLayout = vk::ImageLayout::eColorAttachmentOptimal,
                        .loadOp = vk::AttachmentLoadOp::eLoad,
                        .storeOp = vk::AttachmentStoreOp::eStore,
                    },
                };

                context.command.beginRendering({
                    .renderArea = {
                        .offset = {0, 0},
                        .extent = context.extent,
                    },
                    .layerCount = 1,
                    .colorAttachmentCount = static_cast<std::uint32_t>(colorAttachments.size()),
                    .pColorAttachments = colorAttachments.data(),
                    .pDepthAttachment = {},
                    .pStencilAttachment = {},
                });

                ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), context.command);

                context.command.endRendering();
            },
        });

    return m_Graph.compile();
}

bool ImGuiRenderer::destroy() {
    for (const auto &[resource, stored] : m_Attachments) {
        for (const auto &[viewInfo, array] : stored.textureIds) {
            for (const auto attachment : array) {
                ImGui_ImplVulkan_RemoveTexture(reinterpret_cast<VkDescriptorSet>(attachment));
            }
        }

        resource->eraseCallback<OnAttachmentRebuild>(stored.rebuildId);
    }

    for (const auto &[resource, stored] : m_Textures) {
        for (const auto &[viewInfo, texture] : stored.textureIds) {
            ImGui_ImplVulkan_RemoveTexture(reinterpret_cast<VkDescriptorSet>(texture));
        }

        resource->eraseCallback<OnAttachmentRebuild>(stored.rebuildId);
    }

    ImGui_ImplVulkan_Shutdown();
    ImGui_ImplGlfw_Shutdown();

    ImGui::DestroyContext();
    return true;
}

void ImGuiRenderer::newFrame() {
    ImGui_ImplVulkan_NewFrame();
    ImGui_ImplGlfw_NewFrame();
}

const RenderGraph &ImGuiRenderer::getGraph() const {
    return m_Graph;
}

std::uint32_t ImGuiRenderer::getFrameInFlight() const {
    return m_FrameInFlight;
}

std::optional<ImTextureID> ImGuiRenderer::getTexture(const Resource<Texture> &texture, const Image::ViewInfo &viewInfo) {
    StoredTextureID &stored = m_Textures[texture];

    if (stored.rebuildId == -1) {
        stored.rebuildId = texture->addCallback<OnTextureRebuild>([this, texture](const OnTextureRebuild &event) {
            StoredTextureID &stored = m_Textures[texture];

            for (auto &[viewInfo, textureId] : stored.textureIds) {
                event.context.deletionQueue->enqueue(textureId);

                const Image::Data &data = texture->getData().image.getData();
                if (!data.views.contains(viewInfo)) {
                    continue;
                }

                textureId = createTexture(texture, data.views.at(viewInfo));
            }
        });
    }

    if (!stored.textureIds.contains(viewInfo)) {
        const Image::Data &data = texture->getData().image.getData();
        if (!data.views.contains(viewInfo)) {
            return std::nullopt;
        }

        stored.textureIds[viewInfo] = createTexture(texture, data.views.at(viewInfo));
    }

    return stored.textureIds.at(viewInfo);
}

std::optional<ImTextureID> ImGuiRenderer::getTexture(const Resource<Attachment> &attachment, const Image::ViewInfo &viewInfo) {
    StoredAttachmentID &stored = m_Attachments[attachment];

    if (stored.rebuildId == -1) {
        stored.rebuildId = attachment->addCallback<OnAttachmentRebuild>([this, attachment](const OnAttachmentRebuild &event) {
            StoredAttachmentID &stored = m_Attachments[attachment];

            for (auto &[viewInfo, textureIds] : stored.textureIds) {
                event.context.deletionQueue->enqueue(textureIds[event.context.frameInFlight]);

                const Image::Data &data = attachment->getRingData()[event.context.frameInFlight].image.getData();
                if (!data.views.contains(viewInfo)) {
                    continue;
                }

                textureIds[event.context.frameInFlight] = createTexture(attachment, data.views.at(viewInfo));
            }
        });
    }

    if (!stored.textureIds.contains(viewInfo)) {
        for (std::size_t frameInFlight = 0; frameInFlight < detail::MAX_FRAMES_IN_FLIGHT; frameInFlight++) {
            const Image::Data &data = attachment->getRingData()[frameInFlight].image.getData();

            if (!data.views.contains(viewInfo)) {
                if (frameInFlight > 0) {
                    Logger::WARNING("[ImGuiRenderer] Incomplete attachement views, won't generate ImTextureID");
                }

                return std::nullopt;
            }

            stored.textureIds[viewInfo][frameInFlight] = createTexture(attachment, data.views.at(viewInfo));
        }
    }

    std::shared_ptr<Renderer> renderer = m_Scene->getSystem<Renderer>();
    return stored.textureIds.at(viewInfo)[m_FrameInFlight];
}

ImTextureID ImGuiRenderer::createTexture(const Resource<Texture> &texture, const vk::ImageView &view) {
    vk::Sampler sampler = Builtin::RenderPipelineImGui::ResourceSampler->getData().sampler;
    vk::ImageLayout imageLayout = vk::ImageLayout::eUndefined;

    switch (texture->getInfo().type) {
    case Texture::Type::Dim2D:
    case Texture::Type::Cube:
        imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
        break;
    }

    VkDescriptorSet descriptorSet = ImGui_ImplVulkan_AddTexture(
        static_cast<VkSampler>(sampler),
        static_cast<VkImageView>(view),
        static_cast<VkImageLayout>(imageLayout));

    return reinterpret_cast<ImTextureID>(descriptorSet);
}

ImTextureID ImGuiRenderer::createTexture(const Resource<Attachment> &attachment, const vk::ImageView &view) {
    vk::Sampler sampler = Builtin::RenderPipelineImGui::ResourceSampler->getData().sampler;
    vk::ImageLayout imageLayout = vk::ImageLayout::eUndefined;

    switch (attachment->getInfo().usage) {
    case Attachment::Usage::Color:
        imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
        break;

    case Attachment::Usage::Depth:
        imageLayout = vk::ImageLayout::eDepthReadOnlyOptimal;
        break;

    case Attachment::Usage::Stencil:
        imageLayout = vk::ImageLayout::eStencilReadOnlyOptimal;
        break;

    case Attachment::Usage::DepthStencil:
        imageLayout = vk::ImageLayout::eDepthStencilReadOnlyOptimal;
        break;
    }

    VkDescriptorSet descriptorSet = ImGui_ImplVulkan_AddTexture(
        static_cast<VkSampler>(sampler),
        static_cast<VkImageView>(view),
        static_cast<VkImageLayout>(imageLayout));

    return reinterpret_cast<ImTextureID>(descriptorSet);
}

} // namespace Physbuzz
