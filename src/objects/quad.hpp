#pragma once

#include "builder.hpp"
#include "common.hpp"
#include <physbuzz/physics/dynamics.hpp>

struct QuadComponent {
    float width = 0.0f;
    float height = 0.0f;
};

struct Quad {
    // physics info
    Physbuzz::RigidBodyComponent body;

    // geometry
    Physbuzz::Model model;
    QuadComponent quad;

    // naming
    IdentifiableComponent identifier = {
        .name = "Quad",
    };

    ResourceIdentifierComponent resources = {
        .pipeline = "quad",
    };

    bool isCollidable = false;
    bool isRenderable = false;
};

template <>
Physbuzz::ObjectID ObjectBuilder::create(Physbuzz::ObjectID object, Quad &info);
