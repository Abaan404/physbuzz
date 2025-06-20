#include "renderer.hpp"

#include "../ecs/scene.hpp"
#include "gl/capabilities.hpp"
#include "gl/units.hpp"

namespace Physbuzz {

inline ResourceHandle<ShaderPipelineResource> Passthrough = {"builtin/renderer/passthrough"};

Renderer::Renderer(const RendererInfo &info)
    : m_Info(info), m_Framebuffer(info.framebuffer) {}

bool Renderer::build() {
    bool success = true;

    success &= m_Framebuffer.build();

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

    return true;
}

bool Renderer::destroy() {
    return m_Framebuffer.destroy();
}

void Renderer::tick(Scene &scene) const {
    const Framebuffer *framebuffer = &m_Framebuffer;

    if (m_TargetBuffer) {
        framebuffer = m_TargetBuffer;
    }

    framebuffer->clear();
    framebuffer->bind();

    for (const auto &object : m_Objects) {
        render(scene, object);
    }

    bool depthTest = GL::getCapability(GL::Capabilities::DepthTest);
    GL::setCapability(GL::Capabilities::DepthTest, false);

    GL::TextureUnits::reset();
    framebuffer->bindOutputTexture(m_Info.screenIndex);

    for (const auto &postProcessing : m_Info.postProcessing) {
        // check for reload before binding
        if (!postProcessing->reload()) {
            continue;
        }

        postProcessing->bind();
        postProcessing->setUniform("PBZ_Framebuffer", m_Info.screenIndex);
        postProcessing->draw(scene, -1);
    }

    framebuffer->unbind();

    if (m_TargetBuffer != &m_Framebuffer) {
        Passthrough->bind();
        Passthrough->setUniform("PBZ_Framebuffer", m_Info.screenIndex);
        Passthrough->draw(scene, -1);
    }

    framebuffer->unbindOutputTexture();
    GL::setCapability(GL::Capabilities::DepthTest, depthTest);
}

void Renderer::render(Scene &scene, ObjectID object) const {
    const auto [render] = scene.getComponent<RenderComponent>(object);

    // check for reload before binding
    if (!render.pipeline->reload()) {
        return;
    }

    Physbuzz::GL::TextureUnits::reset();
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
