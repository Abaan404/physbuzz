#include "renderer.hpp"

#include "../ecs/scene.hpp"
#include "gl/capabilities.hpp"
#include "gl/units.hpp"

namespace Physbuzz {

inline ResourceHandle<ShaderPipelineResource> Passthrough = {"builtin/renderer/passthrough"};

Renderer::Renderer(const RendererInfo &info)
    : m_Info(info), m_Framebuffer(info.framebuffer) {}

void Renderer::build() {
    m_Framebuffer.build();

    if (!ResourceRegistry<ModelResource>::contains(ScreenQuad.getIdentifier())) {
        ResourceRegistry<ModelResource>::insert(
            ScreenQuad.getIdentifier(),
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

    if (!ResourceRegistry<ShaderPipelineResource>::contains(Passthrough.getIdentifier())) {
        ResourceRegistry<ShaderPipelineResource>::insert(
            Passthrough.getIdentifier(),
            {{
                .vertex = {.file = {.path = "resources/shaders/builtin/renderer/passthrough.vert"}},
                .tessControl = {},
                .tessEvaluation = {},
                .geometry = {},
                .fragment = {.file = {.path = "resources/shaders/builtin/renderer/passthrough.frag"}},
                .compute = {},
                .draw = [](const ShaderPipelineResource *pipeline, Scene &scene, ObjectID object) {
                    for (const auto &[mesh, _] : ScreenQuad->getMeshs()) {
                        mesh.draw();
                    }
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

    bool depthTest = GL::getCapability(GL::Capabilities::DepthTest);
    GL::setCapability(GL::Capabilities::DepthTest, false);

    GLint unit = GL::TextureUnits::activate();
    glBindTexture(GL_TEXTURE_2D, m_Framebuffer.getColor());

    for (const auto &postProcessing : m_Info.postProcessing) {
        // check for reload before binding
        if (!postProcessing->reload()) {
            continue;
        }

        postProcessing->bind();
        postProcessing->setUniform("PBZ_Framebuffer", unit);
        postProcessing->draw(scene, -1);
    }

    if (m_TargetBuffer) {
        // target framebuffer
        m_TargetBuffer->bind();
    } else {
        // target screen
        m_Framebuffer.unbind();
    }

    Passthrough->bind();
    Passthrough->setUniform("PBZ_Framebuffer", unit);
    Passthrough->draw(scene, -1);

    glBindTexture(GL_TEXTURE_2D, 0);
    GL::TextureUnits::reset();

    GL::setCapability(GL::Capabilities::DepthTest, depthTest);

    if (m_TargetBuffer) {
        m_TargetBuffer->unbind();
    }
}

void Renderer::render(Scene &scene, ObjectID object) {
    const auto [render] = scene.getComponent<RenderComponent>(object);

    // check for reload before binding
    if (!render.pipeline->reload()) {
        return;
    }

    render.pipeline->draw(scene, object);
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
