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
    .diffuse = {0.8f, 0.8f, 0.8f},
    .specular = {1.0f, 1.0f, 1.0f},

    .constant = 0.1f,
    .linear = 0.09f,
    .quadratic = 0.00032f,
};

static Physbuzz::DirectionalLightComponent s_DirectionalLight = {
    .direction = glm::normalize(glm::vec3(1.0f, -1.0f, 1.0f)),

    .ambient = {0.2f, 0.2f, 0.2f},
    .diffuse = {0.5f, 0.5f, 0.5f},
    .specular = {1.0f, 1.0f, 1.0f},
};

static Physbuzz::SpotLightComponent s_SpotLight = {
    .ambient = {0.2f, 0.2f, 0.2f},
    .diffuse = {0.5f, 0.5f, 0.5f},
    .specular = {1.0f, 1.0f, 1.0f},

    .constant = 1.0f,
    .linear = 0.0009f,
    .quadratic = 0.000032f,

    .cutOff = glm::cos(glm::radians(12.5f)),
    .outerCutOff = glm::cos(glm::radians(17.5f)),
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

    // Directional Lighting
    pipeline->setUniform("u_DirectionalLight.direction", s_DirectionalLight.direction);
    pipeline->setUniform("u_DirectionalLight.ambient", s_DirectionalLight.ambient);
    pipeline->setUniform("u_DirectionalLight.diffuse", s_DirectionalLight.diffuse);
    pipeline->setUniform("u_DirectionalLight.specular", s_DirectionalLight.specular);

    // Point Lighting
    pipeline->setUniform("u_PointLight.position", s_PointLight.position);

    pipeline->setUniform("u_PointLight.ambient", s_PointLight.ambient);
    pipeline->setUniform("u_PointLight.diffuse", s_PointLight.diffuse);
    pipeline->setUniform("u_PointLight.specular", s_PointLight.specular);

    pipeline->setUniform("u_PointLight.constant", s_PointLight.constant);
    pipeline->setUniform("u_PointLight.linear", s_PointLight.linear);
    pipeline->setUniform("u_PointLight.quadratic", s_PointLight.quadratic);

    // Spotlight Lighting
    pipeline->setUniform("u_SpotLight.position", activeCamera->view.position);
    pipeline->setUniform("u_SpotLight.direction", activeCamera->view.getFacing());

    pipeline->setUniform("u_SpotLight.ambient", s_SpotLight.ambient);
    pipeline->setUniform("u_SpotLight.diffuse", s_SpotLight.diffuse);
    pipeline->setUniform("u_SpotLight.specular", s_SpotLight.specular);

    pipeline->setUniform("u_SpotLight.constant", s_SpotLight.constant);
    pipeline->setUniform("u_SpotLight.linear", s_SpotLight.linear);
    pipeline->setUniform("u_SpotLight.quadratic", s_SpotLight.quadratic);

    pipeline->setUniform("u_SpotLight.cutOff", s_SpotLight.cutOff);
    pipeline->setUniform("u_SpotLight.outerCutOff", s_SpotLight.outerCutOff);

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
