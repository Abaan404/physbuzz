#pragma once

#include "builder.hpp"
#include <functional>
#include <physbuzz/render/shaders.hpp>
#include <physbuzz/render/texture.hpp>
#include <physbuzz/resources/resources.hpp>

struct IdentifiableComponent {
    std::string name = "Unknown";
    bool hidden = false;
};

struct RebuildableComponent {
    std::function<void(ObjectBuilder &, Physbuzz::ObjectID)> rebuild;
};

struct ResourceComponent {
    std::vector<Physbuzz::Resource<Physbuzz::Texture2D>> textures = {
        {"default/diffuse"},
        {"default/specular"},
    };
    Physbuzz::Resource<Physbuzz::ShaderPipeline> pipeline = {"default"};
};
