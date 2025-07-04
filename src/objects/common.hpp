#pragma once

#include <functional>
#include <physbuzz/render/mesh.hpp>
#include <physbuzz/render/shaders.hpp>
#include <physbuzz/render/texture.hpp>
#include <physbuzz/resources/resources.hpp>

struct IdentifiableComponent {
    std::string name = "Unknown";
    bool hidden = false;
};

struct RebuildableComponent {
    std::function<void(Physbuzz::Scene &, Physbuzz::ObjectID)> rebuild;
};

struct ResourceComponent {
    std::vector<Physbuzz::Resource<Physbuzz::Texture2D>> textures = {
        {"default/diffuse"},
        {"default/specular"},
    };
    Physbuzz::Resource<Physbuzz::ShaderPipeline> pipeline = {"default"};
};

std::vector<glm::vec2> generateTexCoords(const std::vector<glm::vec3> &positions);
std::vector<glm::vec3> generateNormals(const std::vector<Physbuzz::Index> &indices, const std::vector<glm::vec3> &positions);
