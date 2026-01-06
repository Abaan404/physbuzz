#pragma once

#include <functional>
#include <physbuzz/ecs/defines.hpp>
#include <physbuzz/render/shaders.hpp>
#include <physbuzz/render/layouts/texture.hpp>
#include <physbuzz/resources/resources.hpp>

struct IdentifiableComponent {
    std::string name = "Unknown";
    bool hidden = false;
};

struct RebuildableComponent {
    std::function<void(Physbuzz::Scene &, Physbuzz::ObjectID)> rebuild;
};

struct ResourceComponent {
    std::vector<Physbuzz::Resource<Physbuzz::Texture>> textures = {
        {"default/diffuse"},
        {"default/specular"},
    };
    Physbuzz::Resource<Physbuzz::RenderPipeline> pipeline = {"default"};
};
