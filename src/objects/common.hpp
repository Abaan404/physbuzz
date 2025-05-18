#pragma once

#include "builder.hpp"
#include <functional>
#include <physbuzz/render/shaders.hpp>

struct IdentifiableComponent {
    std::string name = "Unknown";
    bool hidden = false;
};

struct RebuildableComponent {
    std::function<void(ObjectBuilder &, Physbuzz::ObjectID)> rebuild;
};

struct ResourceComponent {
    std::unordered_map<Physbuzz::TextureType, std::vector<Physbuzz::ResourceHandle<Physbuzz::Texture2DResource>>> textures = {
        {Physbuzz::TextureType::Diffuse, {{"default/diffuse"}}},
        {Physbuzz::TextureType::Specular, {{"default/specular"}}},
    };
    Physbuzz::ResourceHandle<Physbuzz::ShaderPipelineResource> pipeline = {"default"};
};
