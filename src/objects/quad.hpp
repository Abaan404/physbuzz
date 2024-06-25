#pragma once

#include <physbuzz/dynamics.hpp>
#include "objects.hpp"

struct QuadComponent {
    float width = 0.0f;
    float height = 0.0f;
};

struct QuadInfo {
    // physics info
    Physbuzz::RigidBodyComponent body;

    // geometry
    Physbuzz::TransformableComponent transform;
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
