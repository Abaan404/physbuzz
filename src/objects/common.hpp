#pragma once

#include <physbuzz/ecs/scene.hpp>
#include <physbuzz/graphics/material.hpp>

struct IdentifiableComponent {
    std::string name = "Unknown";
    bool hidden = false;
};

struct ResourceComponent {
    Physbuzz::Resource<Physbuzz::Material> material = {"default"};
};

struct RadialComponent {
    float radius = 0.0f;
};
