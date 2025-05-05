#include "renderer.hpp"
#include "objects/common.hpp"
#include "objects/skybox.hpp"
#include "physbuzz/debug/logging.hpp"
#include "physbuzz/render/cubemap.hpp"
#include "physbuzz/resources/manager.hpp"

#include <cstddef>
#include <format>
#include <physbuzz/render/lighting.hpp>
#include <physbuzz/render/shaders.hpp>
#include <physbuzz/render/texture.hpp>

static Physbuzz::DirectionalLightComponent s_DirectionalLight = {
    .direction = glm::normalize(glm::vec3(1.0f, 1.0f, -1.0f)),

    .ambient = {0.2f, 0.2f, 0.2f},
    .diffuse = {0.5f, 0.5f, 0.5f},
    .specular = {0.5f, 0.5f, 0.5f},
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
    const ShaderComponent &shader = scene.getComponent<ShaderComponent>(object);
    const Physbuzz::TransformComponent &transform = scene.getComponent<Physbuzz::TransformComponent>(object);

    Physbuzz::ShaderPipelineResource *pipeline = Physbuzz::ResourceRegistry::get<Physbuzz::ShaderPipelineResource>(shader.resource);
    PBZ_ASSERT(pipeline, std::format("[Renderer] ShaderPipelineResource '{}' unknown.", shader.resource));

    std::vector<SkyboxComponent> skyboxes = scene.getComponents<SkyboxComponent>();
    PBZ_ASSERT(skyboxes.size() == 1, "[Renderer] Invalid number of skyboxes in scene");

    const SkyboxComponent &skybox = skyboxes[0];
    Physbuzz::CubemapResource *cubemap = Physbuzz::ResourceRegistry::get<Physbuzz::CubemapResource>(skybox.cubemap);

    // check for reload before binding
    if (!pipeline->reload()) {
        return;
    }

    if (!pipeline->bind()) {
        return;
    }

    // skybox
    cubemap->bind();
    pipeline->setUniform("u_Skybox", cubemap->getUnit());

    // time
    pipeline->setUniform<unsigned int>("u_Time", m_Clock.getTime());
    pipeline->setUniform<unsigned int>("u_TimeDelta", m_Clock.getDelta());

    // player
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

    for (std::size_t i = 0; i < pointLights.size(); ++i) {
        pipeline->setUniform(std::format("u_PointLight[{}].position", i), pointLights[i].position);
        pipeline->setUniform(std::format("u_PointLight[{}].ambient", i), pointLights[i].ambient);
        pipeline->setUniform(std::format("u_PointLight[{}].diffuse", i), pointLights[i].diffuse);
        pipeline->setUniform(std::format("u_PointLight[{}].specular", i), pointLights[i].specular);
        pipeline->setUniform(std::format("u_PointLight[{}].constant", i), pointLights[i].constant);
        pipeline->setUniform(std::format("u_PointLight[{}].linear", i), pointLights[i].linear);
        pipeline->setUniform(std::format("u_PointLight[{}].quadratic", i), pointLights[i].quadratic);
    }

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

    // MVP
    pipeline->setUniform("u_Model", transform.matrix);
    pipeline->setUniform("u_View", activeCamera->view.matrix);
    pipeline->setUniform("u_Projection", activeCamera->getProjection());

    shader.render(scene, object);

    // unbind scene
    cubemap->unbind();
    pipeline->unbind();
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
