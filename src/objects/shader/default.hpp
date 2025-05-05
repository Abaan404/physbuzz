#pragma once

#include "../common.hpp"
#include <physbuzz/render/shaders.hpp>
#include <physbuzz/render/texture.hpp>
#include <physbuzz/resources/manager.hpp>
#include <string>
#include <unordered_map>

const ShaderComponent s_DefaultShader = {
    .resource = "default",
    .render = [](Physbuzz::Scene &scene, Physbuzz::ObjectID object) {
        const Physbuzz::ModelComponent &render = scene.getComponent<Physbuzz::ModelComponent>(object);

        const Physbuzz::ModelResource *model = Physbuzz::ResourceRegistry::get<Physbuzz::ModelResource>(render.model);
        PBZ_ASSERT(model, std::format("[Renderer] ModelResource '{}' unknown.", render.model));

        const Physbuzz::ShaderPipelineResource *pipeline = Physbuzz::ResourceRegistry::get<Physbuzz::ShaderPipelineResource>("default");
        PBZ_ASSERT(pipeline, std::format("[Renderer] ShaderPipelineResource '{}' unknown.", "default"));

        // draw meshes
        for (const Physbuzz::Mesh &mesh : model->getMeshs()) {
            if (mesh.textures.contains(Physbuzz::TextureType::Diffuse)) {
                const std::vector<std::string> &diffuseTextures = mesh.textures.at(Physbuzz::TextureType::Diffuse);
                pipeline->setUniform<unsigned int>("u_Material.diffuseLength", diffuseTextures.size());

                for (std::size_t i = 0; i < diffuseTextures.size(); i++) {
                    Physbuzz::Texture2DResource *texture = Physbuzz::ResourceRegistry::get<Physbuzz::Texture2DResource>(diffuseTextures[i]);
                    pipeline->setUniform(std::format("u_MaterialDiffuse[{}]", i), texture->getUnit());
                    texture->bind();
                }
            }

            if (mesh.textures.contains(Physbuzz::TextureType::Specular)) {
                const std::vector<std::string> &specularTextures = mesh.textures.at(Physbuzz::TextureType::Specular);
                pipeline->setUniform<unsigned int>("u_Material.specularLength", specularTextures.size());

                for (std::size_t i = 0; i < specularTextures.size(); i++) {
                    Physbuzz::Texture2DResource *texture = Physbuzz::ResourceRegistry::get<Physbuzz::Texture2DResource>(specularTextures[i]);
                    pipeline->setUniform(std::format("u_MaterialSpecular[{}]", i), texture->getUnit());
                    texture->bind();
                }
            }

            pipeline->setUniform("u_Material.shininess", mesh.shininess);

            mesh.bind();
            mesh.draw();
            mesh.unbind();

            if (mesh.textures.contains(Physbuzz::TextureType::Diffuse)) {
                const std::vector<std::string> &diffuseTextures = mesh.textures.at(Physbuzz::TextureType::Diffuse);

                for (std::size_t i = 0; i < diffuseTextures.size(); i++) {
                    Physbuzz::Texture2DResource *texture = Physbuzz::ResourceRegistry::get<Physbuzz::Texture2DResource>(diffuseTextures[i]);
                    texture->unbind();
                }
            }

            if (mesh.textures.contains(Physbuzz::TextureType::Specular)) {
                const std::vector<std::string> &specularTextures = mesh.textures.at(Physbuzz::TextureType::Specular);

                for (std::size_t i = 0; i < specularTextures.size(); i++) {
                    Physbuzz::Texture2DResource *texture = Physbuzz::ResourceRegistry::get<Physbuzz::Texture2DResource>(specularTextures[i]);
                    texture->unbind();
                }
            }
        }
    },
};
