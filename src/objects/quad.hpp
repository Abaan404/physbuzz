#pragma once

#include "builder.hpp"
#include "common.hpp"
#include <physbuzz/dynamics.hpp>

struct QuadComponent {
    float width = 0.0f;
    float height = 0.0f;
};

struct Quad {
    // physics info
    Physbuzz::RigidBodyComponent body;

    // geometry
    Physbuzz::TransformableComponent transform;
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
Physbuzz::ObjectID ObjectBuilder::create(Physbuzz::Object &object, Quad &info);
