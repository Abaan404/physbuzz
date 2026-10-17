#pragma once

#include <physbuzz/graphics/material.hpp>
#include <physbuzz/graphics/mesh.hpp>
#include <physbuzz/physics/components/rigidbody.hpp>

struct IdentifiableComponent {
    std::string name = "Unknown";
    bool hidden = false;
};

struct ResourceList {
    Physbuzz::Resource<Physbuzz::Mesh> mesh;
    Physbuzz::Resource<Physbuzz::Material> material;

    std::optional<Physbuzz::Resource<Physbuzz::Mesh>> shadowMesh;
};
