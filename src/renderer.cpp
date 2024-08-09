#include "renderer.hpp"

#include "objects/common.hpp"
#include "physbuzz/logging.hpp"
#include <format>
#include <physbuzz/shaders.hpp>

#include <glm/ext/matrix_clip_space.hpp>

Renderer::Renderer(Physbuzz::Window *window)
    : m_Window(window) {}

Renderer::~Renderer() {}

void Renderer::build() {
    target(nullptr);
}

void Renderer::destroy() {}

void Renderer::render(Physbuzz::Scene &scene, Physbuzz::ResourceManager &resources) {
    m_Clock.tick();
    clear(m_ClearColor);

    for (auto &object : scene.getObjects()) {

        if (!object.hasComponent<Physbuzz::Mesh>() || !object.hasComponent<Physbuzz::TransformableComponent>()) {
            continue;
        }

        render(object, resources);
    }
}

void Renderer::render(Physbuzz::Object &object, Physbuzz::ResourceManager &resources) const {
    Physbuzz::Logger::ASSERT(activeCamera != nullptr, "No camera bound to renderer");

    const ResourceIdentifierComponent &identifiers = object.getComponent<ResourceIdentifierComponent>();
    const Physbuzz::ShaderPipelineResource *pipeline = resources.get<Physbuzz::ShaderPipelineResource>(identifiers.pipeline);
    const Physbuzz::Texture2DResource *texture = resources.get<Physbuzz::Texture2DResource>(identifiers.texture2D);

    if (!pipeline) {
        Physbuzz::Logger::WARNING(std::format("[Renderer] ShaderPipelineResource '{}' unknown.", identifiers.pipeline));
        return;
    }

    if (!texture) {
        Physbuzz::Logger::WARNING(std::format("[Renderer] Texture2DResource '{}' unknown.", identifiers.texture2D));
        return;
    }

    const Physbuzz::Mesh &mesh = object.getComponent<Physbuzz::Mesh>();

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
    pipeline->setUniform("u_Model", object.getComponent<Physbuzz::TransformableComponent>().generateModel());
    pipeline->setUniform("u_View", activeCamera->getView());
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
