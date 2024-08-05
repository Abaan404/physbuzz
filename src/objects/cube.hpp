#pragma once

#include "builder.hpp"
#include "common.hpp"
#include <physbuzz/dynamics.hpp>

struct CubeComponent {
    float width = 0.0f;
    float height = 0.0f;
    float length = 0.0f;
};

struct Cube {
    // physics info
    // Physbuzz::RigidBodyComponent body; // TODO implement me

    // geometry
    Physbuzz::TransformableComponent transform;
    CubeComponent cube;

    // naming
    IdentifiableComponent identifier = {
        .name = "Cube",
    };

    bool isCollidable = false;
    bool isRenderable = false;
};

template <>
Physbuzz::ObjectID ObjectBuilder::create(Physbuzz::Object &object, Cube &info);
