#pragma once

#include "../../objects/player.hpp"
#include <physbuzz/render/lighting.hpp>
#include <physbuzz/render/model.hpp>
#include <physbuzz/render/renderer.hpp>
#include <physbuzz/render/shaders.hpp>

inline Physbuzz::ShaderPipelineResource shaderDefault = {{
    .vertex = {.file = {.path = "resources/shaders/default/default.vert"}},
    .tessControl = {},
    .tessEvaluation = {},
    .geometry = {},
    .fragment = {.file = {.path = "resources/shaders/default/default.frag"}},
    .compute = {},
    .draw = [](const Physbuzz::ShaderPipelineResource *pipeline, Physbuzz::Scene &scene, Physbuzz::ObjectID object) {
        const auto [render] = scene.getComponent<Physbuzz::RenderComponent>(object);

        pipeline->setUniform("u_Model", render.transform.matrix);

        static Physbuzz::DirectionalLightComponent directionalLight = {
            .direction = glm::normalize(glm::vec3(1.0f, 1.0f, -1.0f)),

            .ambient = {0.2f, 0.2f, 0.2f},
            .diffuse = {0.5f, 0.5f, 0.5f},
            .specular = {0.5f, 0.5f, 0.5f},
        };

        static Physbuzz::SpotLightComponent spotLight = {
            .ambient = {0.2f, 0.2f, 0.2f},
            .diffuse = {0.5f, 0.5f, 0.5f},
            .specular = {1.0f, 1.0f, 1.0f},

            .constant = 1.0f,
            .linear = 0.0009f,
            .quadratic = 0.000032f,

            .cutOff = glm::radians(12.5f),
            .outerCutOff = glm::radians(17.5f),
        };

        // Directional Lighting
        pipeline->setUniform("u_DirectionalLight.direction", directionalLight.direction);
        pipeline->setUniform("u_DirectionalLight.ambient", directionalLight.ambient);
        pipeline->setUniform("u_DirectionalLight.diffuse", directionalLight.diffuse);
        pipeline->setUniform("u_DirectionalLight.specular", directionalLight.specular);

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

        for (const auto &[_, camera] : scene.getComponents<PlayerComponent, Physbuzz::CameraComponent>()) {
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
            switch (type) {
            case Physbuzz::TextureType::Diffuse:
                pipeline->setUniform(std::format("u_MaterialDiffuse[{}]", textureLengths[type]), texture->getUnit());
                break;

            case Physbuzz::TextureType::Specular:
                pipeline->setUniform(std::format("u_MaterialSpecular[{}]", textureLengths[type]), texture->getUnit());
                break;

            default:
                break;
            }

            textureLengths[type]++;
            texture->bind();
        }

        pipeline->setUniform("u_Material.diffuseLength", textureLengths[Physbuzz::TextureType::Diffuse]);
        pipeline->setUniform("u_Material.specularLength", textureLengths[Physbuzz::TextureType::Specular]);

        for (const auto &[mesh, meta] : render.model->getMeshs()) {
            pipeline->setUniform("u_MaterialShininess", meta.shininess);

            mesh.draw();
        }

        for (const auto &texture : render.model->getTextures()) {
            texture->unbind();
        }
    },
}};
