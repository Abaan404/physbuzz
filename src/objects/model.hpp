#pragma once

#include "common.hpp"
#include <physbuzz/physics/dynamics.hpp>
#include <physbuzz/render/transform.hpp>

struct ModelComponent {
    Physbuzz::ResourceHandle<Physbuzz::ModelResource> resource;
};

struct Model {
    // physics info
    Physbuzz::RigidBodyComponent body;

    // geometry
    ModelComponent model;
    Physbuzz::Transform transform;

    // naming
    IdentifiableComponent identifier = {
        .name = "Generic",
        .hidden = false,
    };

    // rendering
    ResourceComponent resources;

    bool hasPhysics = false;
};

template <>
Physbuzz::ObjectID ObjectBuilder::create(Physbuzz::ObjectID object, Model &info);
