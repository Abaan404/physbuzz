#pragma once

#include "builder.hpp"
#include <functional>
#include <physbuzz/render/model.hpp>

struct IdentifiableComponent {
    std::string name = "Unknown";
    bool hidden = false;
};

struct RebuildableComponent {
    std::function<void(ObjectBuilder &, Physbuzz::ObjectID)> rebuild;
};

struct ResourceComponent {
    std::unordered_map<Physbuzz::TextureType, std::vector<std::string>> textures = {
        {Physbuzz::TextureType::Diffuse, {"default/diffuse"}},
        {Physbuzz::TextureType::Specular, {"default/specular"}},
    };

    std::string pipeline = "default";
};
