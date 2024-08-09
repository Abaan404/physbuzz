#pragma once

#include "builder.hpp"
#include <functional>
#include <physbuzz/object.hpp>

struct IdentifiableComponent {
    std::string name = "Unknown";
    bool hidden = false;
};

struct RebuildableComponent {
    std::function<void(ObjectBuilder &, Physbuzz::Object &)> rebuild;
};

struct ResourceIdentifierComponent {
    std::string pipeline = "default";
    std::string texture2D = "missing";
};
