#include "renderer.hpp"

#include "objects/common.hpp"
#include <glm/ext/matrix_clip_space.hpp>
#include <physbuzz/debug/logging.hpp>
#include <physbuzz/render/lighting.hpp>
#include <physbuzz/render/shaders.hpp>
#include <physbuzz/render/texture.hpp>

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
    const std::vector<Physbuzz::PointLightComponent> &pointLights = scene.getComponents<Physbuzz::PointLightComponent>();
    pipeline->setUniform<unsigned int>("u_PointLightLength", pointLights.size());

    // TODO dont do this
    pipeline->setUniform("u_PointLight[0].position", pointLights[0].position);
    pipeline->setUniform("u_PointLight[0].ambient", pointLights[0].ambient);
    pipeline->setUniform("u_PointLight[0].diffuse", pointLights[0].diffuse);
    pipeline->setUniform("u_PointLight[0].specular", pointLights[0].specular);
    pipeline->setUniform("u_PointLight[0].constant", pointLights[0].constant);
    pipeline->setUniform("u_PointLight[0].linear", pointLights[0].linear);
    pipeline->setUniform("u_PointLight[0].quadratic", pointLights[0].quadratic);

    pipeline->setUniform("u_PointLight[1].position", pointLights[1].position);
    pipeline->setUniform("u_PointLight[1].ambient", pointLights[1].ambient);
    pipeline->setUniform("u_PointLight[1].diffuse", pointLights[1].diffuse);
    pipeline->setUniform("u_PointLight[1].specular", pointLights[1].specular);
    pipeline->setUniform("u_PointLight[1].constant", pointLights[1].constant);
    pipeline->setUniform("u_PointLight[1].linear", pointLights[1].linear);
    pipeline->setUniform("u_PointLight[1].quadratic", pointLights[1].quadratic);

    pipeline->setUniform("u_PointLight[2].position", pointLights[2].position);
    pipeline->setUniform("u_PointLight[2].ambient", pointLights[2].ambient);
    pipeline->setUniform("u_PointLight[2].diffuse", pointLights[2].diffuse);
    pipeline->setUniform("u_PointLight[2].specular", pointLights[2].specular);
    pipeline->setUniform("u_PointLight[2].constant", pointLights[2].constant);
    pipeline->setUniform("u_PointLight[2].linear", pointLights[2].linear);
    pipeline->setUniform("u_PointLight[2].quadratic", pointLights[2].quadratic);

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
