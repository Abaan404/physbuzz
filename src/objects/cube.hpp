#pragma once

#include "builder.hpp"
#include "common.hpp"
#include <physbuzz/render/transform.hpp>

struct CubeComponent {
    float width = 0.0f;
    float height = 0.0f;
    float length = 0.0f;
};

struct Cube {
    // physics info
    // Physbuzz::RigidBodyComponent body; // TODO implement me

    // geometry
    CubeComponent cube;
    Physbuzz::TransformComponent transform;

    // naming
    IdentifiableComponent identifier = {
        .name = "Cube",
        .hidden = false,
    };

    // rendering
    ResourceComponent resources = {
        .pipeline = "cube",
    };

    bool hasPhysics = false;
};

template <>
Physbuzz::ObjectID ObjectBuilder::create(Physbuzz::ObjectID object, Cube &info);
