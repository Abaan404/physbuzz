#pragma once

#include "builder.hpp"
#include <functional>
#include <physbuzz/object.hpp>

enum class ObjectType {
    Circle,
    Quad,
    Line,
    Wall,
    Unknown,
};

struct IdentifiableComponent {
    ObjectType type = ObjectType::Unknown;
    std::string name = "Unknown";
    bool hidden = false;
};

struct RebuildableComponent {
    std::function<void(ObjectBuilder &, Physbuzz::Object &)> rebuild;
};

struct TextureComponent {
    std::string identifier;
};
