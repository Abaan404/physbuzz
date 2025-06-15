#include "renderer.hpp"

#include "../render/model.hpp"
#include "../render/shaders.hpp"
#include "../resources/handle.hpp"
#include "physbuzz/render/gl/capabilities.hpp"
#include <glad/gl.h>

namespace Physbuzz {

inline ResourceID passthroughID = "builtin/renderer/passthrough";

Renderer::Renderer(const RendererInfo &info)
    : m_Info(info), m_Framebuffer(info.framebuffer) {}

void Renderer::build() {
    m_Framebuffer.build();

    if (!ResourceRegistry<ModelResource>::contains(passthroughID)) {
        ResourceRegistry<ModelResource>::insert(
            passthroughID,
            {{
                .meshes = {
                    {
                        {{
                            .vertices = {
                                {{-1.0f, -1.0f, 0.0f}, {}, {0.0f, 0.0f}},
                                {{1.0f, -1.0f, 0.0f}, {}, {1.0f, 0.0f}},
                                {{1.0f, 1.0f, 0.0f}, {}, {1.0f, 1.0f}},
                                {{-1.0f, 1.0f, 0.0f}, {}, {0.0f, 1.0f}},
                            },
                            .indices = {{0, 1, 2, 2, 3, 0}},
                        }},
                        {},
                    },
                },
            }});
    }

    if (!ResourceRegistry<ShaderPipelineResource>::contains(passthroughID)) {
        ResourceRegistry<ShaderPipelineResource>::insert(
            passthroughID,
            {{
                .vertex = {.file = {.path = "resources/shaders/builtin/renderer/passthrough.vert"}},
                .tessControl = {},
                .tessEvaluation = {},
                .geometry = {},
                .fragment = {.file = {.path = "resources/shaders/builtin/renderer/passthrough.frag"}},
                .compute = {},
                .draw = [](const ShaderPipelineResource *resource, Scene &scene, ObjectID object) {
                    const ResourceHandle<ShaderPipelineResource> pipeline = {passthroughID};

                    bool depthTest = GL::getCapability(GL::Capabilities::DepthTest);
                    GL::setCapability(GL::Capabilities::DepthTest, false);

                    glActiveTexture(GL_TEXTURE0);
                    glBindTexture(GL_TEXTURE_2D, scene.getSystem<Renderer>()->getFramebuffer().getColor());
                    pipeline->setUniform("u_ScreenTexture", 0);

                    for (const auto &[mesh, _] : ResourceHandle<ModelResource>(passthroughID)->getMeshs()) {
                        mesh.draw();
                    }

                    glBindTexture(GL_TEXTURE_2D, 0);

                    GL::setCapability(GL::Capabilities::DepthTest, depthTest);
                },
            }});
    }
}

void Renderer::destroy() {
    m_Framebuffer.destroy();
}

void Renderer::tick(Scene &scene) {
    m_Framebuffer.bind();
    m_Framebuffer.clear();

    for (const auto &object : m_Objects) {
        render(scene, object);
    }

    if (m_TargetBuffer) {
        m_TargetBuffer->bind();
    } else {
        // unbound framebuffer targets screen
        m_Framebuffer.unbind();
    }

    // render to screen
    ResourceHandle<ShaderPipelineResource>(passthroughID)->draw(scene, -1);

    if (m_TargetBuffer) {
        m_TargetBuffer->unbind();
    }
}

void Renderer::render(Scene &scene, ObjectID object) {
    const auto [render] = scene.getComponent<RenderComponent>(object);

    for (const auto &pipeline : render.renderpasses) {
        // check for reload before binding
        if (!pipeline->reload()) {
            continue;
        }

        pipeline->draw(scene, object);
    }
}

void Renderer::resize(const glm::ivec2 &resolution) {
    m_Framebuffer.bind();
    m_Framebuffer.resize(resolution);
    m_Framebuffer.unbind();
}

void Renderer::target(const Framebuffer *framebuffer) {
    m_TargetBuffer = framebuffer;
}

const Framebuffer &Renderer::getFramebuffer() {
    return m_Framebuffer;
}

} // namespace Physbuzz
