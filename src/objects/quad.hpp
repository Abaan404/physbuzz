#pragma once

#include "../dynamics/dynamics.hpp"
#include "objects.hpp"

struct QuadComponent {
    float width = 0.0f;
    float height = 0.0f;
};

struct QuadInfo {
    // physics info
    TransformableComponent transform;
    RigidBodyComponent body;

    // geometry
    QuadComponent quad;

    // naming
    IdentifiableComponent identifier = {
        .type = ObjectType::Quad,
        .name = "Quad",
    };

    bool isCollidable = false;
    bool isRenderable = false;
};

template <>
Physbuzz::ObjectID ObjectBuilder<QuadInfo>::build(Physbuzz::Object &object, QuadInfo &info);
