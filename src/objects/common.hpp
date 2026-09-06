#pragma once

#include <physbuzz/graphics/material.hpp>
#include <physbuzz/graphics/mesh.hpp>
#include <physbuzz/physics/components/rigidbody.hpp>

struct IdentifiableComponent {
    std::string name = "Unknown";
    bool hidden = false;
};

struct ResourceComponent {
    Physbuzz::Resource<Physbuzz::Mesh> mesh = {"default"};
    Physbuzz::Resource<Physbuzz::Material> material = {"default"};
};
