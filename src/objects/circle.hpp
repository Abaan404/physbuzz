#pragma once

#include <physbuzz/dynamics.hpp>
#include "objects.hpp"

struct CircleComponent {
    float radius = 0.0f;
};

struct CircleInfo {
    // physics info
    Physbuzz::RigidBodyComponent body;

    // geometry
    Physbuzz::TransformableComponent transform;
    CircleComponent circle;

    // naming
    IdentifiableComponent identifier = {
        .type = ObjectType::Circle,
        .name = "Circle",
    };

    bool isCollidable = false;
    bool isRenderable = false;
};

template <>
Physbuzz::ObjectID ObjectBuilder<CircleInfo>::build(Physbuzz::Object &object, CircleInfo &info);
