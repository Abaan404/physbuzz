#pragma once

#include "common.hpp"
#include <physbuzz/physics/dynamics.hpp>
#include <physbuzz/render/model.hpp>
#include <physbuzz/resources/manager.hpp>

struct CircleComponent {
    float radius = 0.0f;
};

struct Circle {
    // physics info
    Physbuzz::RigidBodyComponent body;

    // geometry
    CircleComponent circle;
    Physbuzz::TransformComponent transform;

    // naming
    IdentifiableComponent identifier = {
        .name = "Circle",
        .hidden = false,
    };

    // resources
    ResourceComponent resources = {
        .pipeline = "circle",
    };

    bool hasPhysics = false;
};

template <>
Physbuzz::ObjectID ObjectBuilder::create(Physbuzz::ObjectID object, Circle &info);
