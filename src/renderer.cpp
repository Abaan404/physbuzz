#include "renderer.hpp"
#include "objects/common.hpp"
#include "objects/skybox.hpp"
#include "resources/uniforms/camera.hpp"
#include "resources/uniforms/time.hpp"
#include "resources/uniforms/window.hpp"

#include <cstddef>
#include <format>
#include <physbuzz/render/cubemap.hpp>
#include <physbuzz/render/lighting.hpp>
#include <physbuzz/render/shaders.hpp>
#include <physbuzz/render/texture.hpp>
#include <physbuzz/render/uniforms.hpp>

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

    Physbuzz::ResourceHandle<Physbuzz::UniformBufferResource<UniformCamera>>("camera")->update({
        .position = activeCamera->view.position,
        .view = activeCamera->view.matrix,
        .projection = activeCamera->getProjection(),
    });

    Physbuzz::ResourceHandle<Physbuzz::UniformBufferResource<UniformTime>>("time")->update({
        .time = m_Clock.getTime(),
        .timedelta = m_Clock.getDelta(),
    });

    for (const auto &object : m_Objects) {
        render(scene, object);
    }
}

void Renderer::render(Physbuzz::Scene &scene, Physbuzz::ObjectID object) {
    const ResourceComponent &resources = scene.getComponent<ResourceComponent>(object);
    const Physbuzz::TransformComponent &transform = scene.getComponent<Physbuzz::TransformComponent>(object);
    std::vector<SkyboxComponent> skyboxes = scene.getComponents<SkyboxComponent>();

    const SkyboxComponent &skybox = skyboxes[0]; // fetch first skybox

    // check for reload before binding
    if (!resources.pipeline->reload()) {
        return;
    }

    if (!resources.pipeline->bind()) {
        return;
    }

    // skybox
    skybox.cubemap->bind();

    resources.pipeline->setUniform("u_Skybox", skybox.cubemap->getUnit());

    // Directional Lighting
    resources.pipeline->setUniform("u_DirectionalLight.direction", s_DirectionalLight.direction);
    resources.pipeline->setUniform("u_DirectionalLight.ambient", s_DirectionalLight.ambient);
    resources.pipeline->setUniform("u_DirectionalLight.diffuse", s_DirectionalLight.diffuse);
    resources.pipeline->setUniform("u_DirectionalLight.specular", s_DirectionalLight.specular);

    // Point Lighting
    const std::vector<Physbuzz::PointLightComponent> &pointLights = scene.getComponents<Physbuzz::PointLightComponent>();
    resources.pipeline->setUniform<unsigned int>("u_PointLightLength", pointLights.size());

    for (std::size_t i = 0; i < pointLights.size(); ++i) {
        resources.pipeline->setUniform(std::format("u_PointLight[{}].position", i), pointLights[i].position);
        resources.pipeline->setUniform(std::format("u_PointLight[{}].ambient", i), pointLights[i].ambient);
        resources.pipeline->setUniform(std::format("u_PointLight[{}].diffuse", i), pointLights[i].diffuse);
        resources.pipeline->setUniform(std::format("u_PointLight[{}].specular", i), pointLights[i].specular);
        resources.pipeline->setUniform(std::format("u_PointLight[{}].constant", i), pointLights[i].constant);
        resources.pipeline->setUniform(std::format("u_PointLight[{}].linear", i), pointLights[i].linear);
        resources.pipeline->setUniform(std::format("u_PointLight[{}].quadratic", i), pointLights[i].quadratic);
    }

    // Spotlight Lighting
    resources.pipeline->setUniform("u_SpotLight.position", activeCamera->view.position);
    resources.pipeline->setUniform("u_SpotLight.direction", activeCamera->view.getFacing());

    resources.pipeline->setUniform("u_SpotLight.ambient", s_SpotLight.ambient);
    resources.pipeline->setUniform("u_SpotLight.diffuse", s_SpotLight.diffuse);
    resources.pipeline->setUniform("u_SpotLight.specular", s_SpotLight.specular);

    resources.pipeline->setUniform("u_SpotLight.constant", s_SpotLight.constant);
    resources.pipeline->setUniform("u_SpotLight.linear", s_SpotLight.linear);
    resources.pipeline->setUniform("u_SpotLight.quadratic", s_SpotLight.quadratic);

    resources.pipeline->setUniform("u_SpotLight.cutOff", s_SpotLight.cutOff);
    resources.pipeline->setUniform("u_SpotLight.outerCutOff", s_SpotLight.outerCutOff);

    // MVP
    resources.pipeline->setUniform("u_Model", transform.matrix);

    resources.pipeline->draw(scene, object);

    // unbind scene
    skybox.cubemap->unbind();
    resources.pipeline->unbind();
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
        Physbuzz::ResourceHandle<Physbuzz::UniformBufferResource<UniformWindow>>("window")->update({
            .resolution = resolution,
        });

    } else {
        m_Framebuffer->resize(resolution);
    }

    m_Resolution = resolution;
    glViewport(0, 0, resolution.x, resolution.y);
}
