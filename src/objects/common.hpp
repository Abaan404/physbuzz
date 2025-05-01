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

struct TextureResources {
    std::unordered_map<Physbuzz::TextureType, std::vector<std::string>> texture2D;
};
