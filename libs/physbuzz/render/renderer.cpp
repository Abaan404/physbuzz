#include "renderer.hpp"

#include "../ecs/scene.hpp"
#include "../resources/builtins/meshes.hpp"
#include "../resources/builtins/shaders.hpp"
#include "gl/capabilities.hpp"
#include "gl/units.hpp"

namespace Physbuzz {

Renderer::Renderer(const RendererInfo &info)
    : m_Info(info), m_Framebuffer(info.framebuffer) {}

bool Renderer::build() {
    bool success = true;

    success &= m_Framebuffer.build();
    success &= Builtin::ScreenQuad::build();
    success &= Builtin::Passthrough::build();

    return success;
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
        if (!postProcessing->reload()) {
            continue;
        }

        postProcessing->bind();
        postProcessing->setUniform("PBZ_Framebuffer", m_Info.screenIndex);
        postProcessing->draw(scene, -1);
        postProcessing->unbind();
    }

    framebuffer->unbind();

    if (framebuffer == &m_Framebuffer && Builtin::Passthrough::Resource->reload()) {
        Builtin::Passthrough::Resource->bind();
        Builtin::Passthrough::Resource->setUniform("PBZ_Framebuffer", m_Info.screenIndex);
        Builtin::Passthrough::Resource->draw(scene, -1);
        Builtin::Passthrough::Resource->unbind();
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

    GL::TextureUnits::reset();
    render.pipeline->bind();
    render.pipeline->draw(scene, object);
    render.pipeline->unbind();
}

void Renderer::resize(const glm::ivec2 &resolution) {
    m_Framebuffer.bind();
    m_Framebuffer.resize(resolution);
    m_Framebuffer.unbind();
}

void Renderer::target(const Framebuffer *framebuffer) {
    m_TargetBuffer = framebuffer;

    const glm::ivec2 &resolution = framebuffer ? framebuffer->getInfo().resolution : m_Framebuffer.getInfo().resolution;
    glViewport(0, 0, resolution.x, resolution.y);
}

const Framebuffer &Renderer::getFramebuffer() const {
    return m_Framebuffer;
}

} // namespace Physbuzz
