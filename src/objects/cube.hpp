#pragma once

#include "builder.hpp"
#include "common.hpp"

struct CubeComponent {
    float width = 0.0f;
    float height = 0.0f;
    float length = 0.0f;
};

struct Cube {
    // physics info
    // Physbuzz::RigidBodyComponent body; // TODO implement me

    // geometry
    Physbuzz::Model model;
    CubeComponent cube;

    // naming
    IdentifiableComponent identifier = {
        .name = "Cube",
    };

    ResourceIdentifierComponent resources = {
        .pipeline = "cube",
    };

    bool isCollidable = false;
    bool isRenderable = false;
};

template <>
Physbuzz::ObjectID ObjectBuilder::create(Physbuzz::ObjectID object, Cube &info);
