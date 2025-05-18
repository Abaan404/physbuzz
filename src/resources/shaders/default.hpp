#pragma once

#include <physbuzz/render/model.hpp>
#include <physbuzz/render/shaders.hpp>

inline Physbuzz::ShaderPipelineResource shaderDefault = {{
    .vertex = {.file = {.path = "resources/shaders/default/default.vert"}},
    .fragment = {.file = {.path = "resources/shaders/default/default.frag"}},
    .draw = [](Physbuzz::Scene &scene, Physbuzz::ObjectID object) {
        const Physbuzz::ModelComponent &render = scene.getComponent<Physbuzz::ModelComponent>(object);
        const Physbuzz::ResourceHandle<Physbuzz::ShaderPipelineResource> pipeline = {"default"};

        // draw meshes
        for (const Physbuzz::Mesh &mesh : render.model->getMeshs()) {
            if (mesh.textures.contains(Physbuzz::TextureType::Diffuse)) {
                const std::vector<Physbuzz::ResourceHandle<Physbuzz::Texture2DResource>> &textures = mesh.textures.at(Physbuzz::TextureType::Diffuse);
                pipeline->setUniform<unsigned int>("u_Material.diffuseLength", textures.size());

                for (std::size_t i = 0; i < textures.size(); i++) {
                    pipeline->setUniform(std::format("u_MaterialDiffuse[{}]", i), textures[i]->getUnit());
                    textures[i]->bind();
                }
            }

            if (mesh.textures.contains(Physbuzz::TextureType::Specular)) {
                const std::vector<Physbuzz::ResourceHandle<Physbuzz::Texture2DResource>> &textures = mesh.textures.at(Physbuzz::TextureType::Specular);
                pipeline->setUniform<unsigned int>("u_Material.specularLength", textures.size());

                for (std::size_t i = 0; i < textures.size(); i++) {
                    pipeline->setUniform(std::format("u_MaterialSpecular[{}]", i), textures[i]->getUnit());
                    textures[i]->bind();
                }
            }

            pipeline->setUniform("u_Material.shininess", mesh.shininess);

            mesh.bind();
            mesh.draw();
            mesh.unbind();

            if (mesh.textures.contains(Physbuzz::TextureType::Diffuse)) {
                for (const auto &texture : mesh.textures.at(Physbuzz::TextureType::Diffuse)) {
                    texture->unbind();
                }
            }

            if (mesh.textures.contains(Physbuzz::TextureType::Specular)) {
                for (const auto &texture : mesh.textures.at(Physbuzz::TextureType::Specular)) {
                    texture->unbind();
                }
            }
        }
    },
}};
