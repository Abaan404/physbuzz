#pragma once

#include "builder.hpp"
#include <functional>

struct IdentifiableComponent {
    std::string name = "Unknown";
    bool hidden = false;
};

struct RebuildableComponent {
    std::function<void(ObjectBuilder &, Physbuzz::ObjectID)> rebuild;
};

struct ResourceIdentifierComponent {
    std::string pipeline = "default";
    std::string texture2D = "missing";
};
