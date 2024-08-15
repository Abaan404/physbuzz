#include "renderer.hpp"

#include "objects/common.hpp"
#include <glm/ext/matrix_clip_space.hpp>
#include <physbuzz/debug/logging.hpp>
#include <physbuzz/render/shaders.hpp>
#include <physbuzz/render/texture.hpp>

Renderer::Renderer(Physbuzz::Window *window, Physbuzz::ResourceManager *resources)
    : m_Window(window), m_Resources(resources) {}

Renderer::~Renderer() {}

void Renderer::build() {
    target(nullptr);
}

void Renderer::destroy() {}

void Renderer::tick(Physbuzz::Scene &scene) {
    m_Clock.tick();
    clear(m_ClearColor);

    if (!activeCamera) {
        Physbuzz::Logger::ERROR("[Renderer] No camera bound to renderer.");
        return;
    }

    for (const auto &object : m_Objects) {
        render(scene, object);
    }
}

void Renderer::render(Physbuzz::Scene &scene, Physbuzz::ObjectID object) {
    const ResourceIdentifierComponent &identifiers = scene.getComponent<ResourceIdentifierComponent>(object);
    const Physbuzz::ShaderPipelineResource *pipeline = m_Resources->get<Physbuzz::ShaderPipelineResource>(identifiers.pipeline);
    const Physbuzz::Texture2DResource *texture = m_Resources->get<Physbuzz::Texture2DResource>(identifiers.texture2D);

    if (!pipeline) {
        Physbuzz::Logger::ERROR("[Renderer] ShaderPipelineResource '{}' unknown.", identifiers.pipeline);
        return;
    }

    if (!texture) {
        Physbuzz::Logger::WARNING("[Renderer] Texture2DResource '{}' unknown.", identifiers.texture2D);
        texture = m_Resources->get<Physbuzz::Texture2DResource>("missing");
    }

    const Physbuzz::MeshComponent &mesh = scene.getComponent<Physbuzz::MeshComponent>(object);

    pipeline->bind();
    texture->bind();
    mesh.bind();

    // texture sampler
    pipeline->setUniform("u_Texture", texture->getUnit());

    // time info
    pipeline->setUniform<unsigned int>("u_Time", m_Clock.getTime());
    pipeline->setUniform<unsigned int>("u_TimeDelta", m_Clock.getDelta());

    // render info
    pipeline->setUniform("u_Resolution", m_Window->getResolution());

    // MVP camera info
    pipeline->setUniform("u_Model", mesh.model.matrix);
    pipeline->setUniform("u_View", activeCamera->view.matrix);
    pipeline->setUniform("u_Projection", activeCamera->getProjection());

    // send draw call
    mesh.draw();

    mesh.unbind();
    pipeline->unbind();
    texture->unbind();
}

const Physbuzz::Clock &Renderer::getClock() const {
    return m_Clock;
}

void Renderer::target(Physbuzz::Framebuffer *framebuffer) {
    m_Framebuffer = framebuffer;

    if (framebuffer) {
        framebuffer->bind();
        resize(framebuffer->getResolution());
    } else {
        framebuffer->unbind();
        resize(m_Window->getResolution());
    }
}

void Renderer::clear(const glm::vec4 &color) {
    m_Framebuffer->clear(color);
}

void Renderer::resize(const glm::ivec2 &resolution) {
    if (m_Framebuffer == nullptr) {
        m_Window->setResolution(resolution);
    } else {
        m_Framebuffer->resize(resolution);
    }

    m_Resolution = resolution;
    glViewport(0, 0, resolution.x, resolution.y);
}
