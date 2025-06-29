#pragma once

#include "common.hpp"
#include <physbuzz/physics/dynamics.hpp>

struct CircleComponent {
    float radius = 0.0f;
};

struct Circle {
    // physics info
    Physbuzz::RigidBodyComponent body;

    // geometry
    CircleComponent circle;
    Physbuzz::Transform transform;

    // naming
    IdentifiableComponent identifier = {
        .name = "Circle",
        .hidden = false,
    };

    // resources
    ResourceComponent resources;

    bool hasPhysics = false;
};

template <>
Physbuzz::ObjectID ObjectBuilder::create(Physbuzz::ObjectID object, Circle &info);
