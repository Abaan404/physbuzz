#pragma once

#include "builder.hpp"
#include <functional>
#include <physbuzz/render/shaders.hpp>
#include <physbuzz/render/texture.hpp>
#include <physbuzz/resources/handle.hpp>

struct IdentifiableComponent {
    std::string name = "Unknown";
    bool hidden = false;
};

struct RebuildableComponent {
    std::function<void(ObjectBuilder &, Physbuzz::ObjectID)> rebuild;
};

struct ResourceComponent {
    std::vector<Physbuzz::ResourceHandle<Physbuzz::Texture2DResource>> textures = {
        {"default/diffuse"},
        {"default/specular"},
    };
    Physbuzz::ResourceHandle<Physbuzz::ShaderPipelineResource> pipeline = {"default"};
};
