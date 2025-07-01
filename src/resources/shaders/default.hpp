#pragma once

#include "../../objects/player.hpp"
#include <physbuzz/render/gl/units.hpp>
#include <physbuzz/render/lighting.hpp>
#include <physbuzz/render/renderer.hpp>
#include <physbuzz/render/shadow.hpp>

inline Physbuzz::ShaderPipeline shaderDefault = {{
    .vertex = {.file = {.path = "resources/shaders/default/default.vert"}},
    .tessControl = {},
    .tessEvaluation = {},
    .geometry = {},
    .fragment = {.file = {.path = "resources/shaders/default/default.frag"}},
    .compute = {},
    .draw = [](const Physbuzz::ShaderPipeline *pipeline, Physbuzz::Scene &scene, Physbuzz::ObjectID object) {
        const auto [render] = scene.getComponent<Physbuzz::RenderComponent>(object);
        const auto &shadow = scene.getSystem<Physbuzz::Shadow>();

        pipeline->setUniform("u_Model", render.transform.matrix);

        // Directional Lighting
        const auto &directionalLights = scene.getComponents<Physbuzz::DirectionalLightComponent>();
        pipeline->setUniform<unsigned int>("u_DirectionalLightLength", directionalLights.size());

        for (std::size_t i = 0; i < directionalLights.size(); ++i) {
            const auto &[directional] = directionalLights[i];

            pipeline->setUniform(std::format("u_DirectionalLight[{}].direction", i), directional.direction);
            pipeline->setUniform(std::format("u_DirectionalLight[{}].ambient", i), directional.ambient);
            pipeline->setUniform(std::format("u_DirectionalLight[{}].diffuse", i), directional.diffuse);
            pipeline->setUniform(std::format("u_DirectionalLight[{}].specular", i), directional.specular);
            pipeline->setUniform(std::format("u_DirectionalLight[{}].matrix", i), directional.matrix);
        }

        // Point Lighting
        const auto &pointLights = scene.getComponents<Physbuzz::PointLightComponent>();
        pipeline->setUniform<unsigned int>("u_PointLightLength", pointLights.size());

        for (std::size_t i = 0; i < pointLights.size(); ++i) {
            const auto &[pointLight] = pointLights[i];

            pipeline->setUniform(std::format("u_PointLight[{}].position", i), pointLight.position);
            pipeline->setUniform(std::format("u_PointLight[{}].ambient", i), pointLight.ambient);
            pipeline->setUniform(std::format("u_PointLight[{}].diffuse", i), pointLight.diffuse);
            pipeline->setUniform(std::format("u_PointLight[{}].specular", i), pointLight.specular);
            pipeline->setUniform(std::format("u_PointLight[{}].constant", i), pointLight.constant);
            pipeline->setUniform(std::format("u_PointLight[{}].linear", i), pointLight.linear);
            pipeline->setUniform(std::format("u_PointLight[{}].quadratic", i), pointLight.quadratic);
        }

        for (const auto &[_, camera, spotLight] : scene.getComponents<PlayerComponent, Physbuzz::CameraComponent, Physbuzz::SpotLightComponent>()) {
            // Spotlight Lighting
            pipeline->setUniform("u_SpotLight.position", camera.getInfo().view.position);
            pipeline->setUniform("u_SpotLight.direction", camera.getFacing());

            pipeline->setUniform("u_SpotLight.ambient", spotLight.ambient);
            pipeline->setUniform("u_SpotLight.diffuse", spotLight.diffuse);
            pipeline->setUniform("u_SpotLight.specular", spotLight.specular);

            pipeline->setUniform("u_SpotLight.constant", spotLight.constant);
            pipeline->setUniform("u_SpotLight.linear", spotLight.linear);
            pipeline->setUniform("u_SpotLight.quadratic", spotLight.quadratic);

            pipeline->setUniform("u_SpotLight.cutOff", glm::cos(spotLight.cutOff));
            pipeline->setUniform("u_SpotLight.outerCutOff", glm::cos(spotLight.outerCutOff));
        }

        std::unordered_map<Physbuzz::TextureType, std::uint32_t> textureLengths;

        for (const auto &texture : render.model->getTextures()) {
            const Physbuzz::TextureType type = texture->getType();
            const std::string name = render.model->getTextureTypeName(type);

            pipeline->setUniform(std::format("u_Material{}[{}]", name, textureLengths[type]), Physbuzz::GL::TextureUnits::activate());
            texture->bind();
            textureLengths[type]++;
        }

        for (const auto [type, size] : textureLengths) {
            pipeline->setUniform(std::format("u_Material{}Length", render.model->getTextureTypeName(type)), size);
        }

        pipeline->setUniform("u_Material.diffuseLength", textureLengths[Physbuzz::TextureType::Diffuse]);
        pipeline->setUniform("u_Material.specularLength", textureLengths[Physbuzz::TextureType::Specular]);

        // shadow map
        const Physbuzz::Shadow::Framebuffers &shadowMaps = shadow->getFramebuffers();

        pipeline->setUniform("u_ShadowMapDirectional", Physbuzz::GL::TextureUnits::activate());
        shadowMaps.directional.bindOutputTexture();

        pipeline->setUniform("u_ShadowMapPoint", Physbuzz::GL::TextureUnits::activate());
        shadowMaps.point.bindOutputTexture();

        // shadow far plane
        pipeline->setUniform("u_FarPlane", shadow->getInfo().depth);

        for (const auto &[mesh, meta] : render.model->getMeshs()) {
            pipeline->setUniform("u_MaterialShininess", meta.shininess);

            mesh.draw();
        }

        for (const auto &texture : render.model->getTextures()) {
            texture->unbind();
        }
    },
}};
