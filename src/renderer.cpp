#include "renderer.hpp"

#include "objects/common.hpp"
#include <glm/ext/matrix_clip_space.hpp>
#include <physbuzz/debug/logging.hpp>
#include <physbuzz/render/lighting.hpp>
#include <physbuzz/render/shaders.hpp>
#include <physbuzz/render/texture.hpp>

static Physbuzz::PointLightComponent s_PointLight = {
    .position = {100.0f, 100.0f, 100.0f},
    .ambient = {0.2f, 0.2f, 0.2f},
    .diffuse = {0.5f, 0.5f, 0.5f},
    .specular = {1.0f, 1.0f, 1.0f},
};

Renderer::Renderer(Physbuzz::Window *window)
    : m_Window(window) {}

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

    Physbuzz::ShaderPipelineResource *pipeline = Physbuzz::ResourceRegistry::get<Physbuzz::ShaderPipelineResource>(identifiers.pipeline);
    if (!pipeline) {
        Physbuzz::Logger::ERROR("[Renderer] ShaderPipelineResource '{}' unknown.", identifiers.pipeline);
        return;
    }

    // check for reload before binding
    if (!pipeline->reload()) {
        return;
    }

    if (!pipeline->bind()) {
        pipeline->unbind();
        return;
    }

    const Physbuzz::MeshComponent &mesh = scene.getComponent<Physbuzz::MeshComponent>(object);

    // fetch materials
    Physbuzz::Texture2DResource *diffuse = Physbuzz::ResourceRegistry::get<Physbuzz::Texture2DResource>(mesh.material.diffuse);
    Physbuzz::Texture2DResource *specular = Physbuzz::ResourceRegistry::get<Physbuzz::Texture2DResource>(mesh.material.specular);
    if (diffuse == nullptr) {
        diffuse = Physbuzz::ResourceRegistry::get<Physbuzz::Texture2DResource>("missing");
    }

    if (specular == nullptr) {
        specular = Physbuzz::ResourceRegistry::get<Physbuzz::Texture2DResource>("missing_specular");
    }

    diffuse->bind();
    specular->bind();
    mesh.bind();

    // time
    pipeline->setUniform<unsigned int>("u_Time", m_Clock.getTime());
    pipeline->setUniform<unsigned int>("u_TimeDelta", m_Clock.getDelta());

    // render
    pipeline->setUniform("u_Resolution", m_Window->getResolution());
    pipeline->setUniform("u_ViewPosition", activeCamera->view.position);

    // Lighting
    pipeline->setUniform("u_Light.position", s_PointLight.position);
    pipeline->setUniform("u_Light.ambient", s_PointLight.ambient);
    pipeline->setUniform("u_Light.diffuse", s_PointLight.diffuse);
    pipeline->setUniform("u_Light.specular", s_PointLight.specular);

    // Material
    pipeline->setUniform("u_Material.diffuse", diffuse->getUnit());
    pipeline->setUniform("u_Material.specular", specular->getUnit());
    pipeline->setUniform("u_Material.shininess", mesh.material.shininess);

    // MVP
    pipeline->setUniform("u_Model", mesh.model.matrix);
    pipeline->setUniform("u_View", activeCamera->view.matrix);
    pipeline->setUniform("u_Projection", activeCamera->getProjection());

    // send draw call
    mesh.draw();

    mesh.unbind();
    pipeline->unbind();
    diffuse->unbind();
    specular->unbind();
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
