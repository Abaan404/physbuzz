#pragma once

#include <functional>
#include <physbuzz/ecs/scene.hpp>
#include <physbuzz/graphics/material.hpp>

struct IdentifiableComponent {
    std::string name = "Unknown";
    bool hidden = false;
};

struct RebuildableComponent {
    std::function<void(Physbuzz::Scene &, Physbuzz::ObjectID)> rebuild;
};

struct ResourceComponent {
    Physbuzz::Resource<Physbuzz::Material> material = {""};
};
