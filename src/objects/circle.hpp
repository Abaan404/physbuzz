#pragma once

#include "builder.hpp"
#include "common.hpp"
#include <physbuzz/physics/dynamics.hpp>

struct RadialComponent {
    float radius = 0.0f;
};

struct Circle {
    // physics info
    Physbuzz::RigidBodyComponent body;

    // geometry
    RadialComponent circle;
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
struct IsBuildable<Circle> : std::true_type {};

template <>
Physbuzz::ObjectID ObjectBuilder::create(Physbuzz::Scene &scene, Physbuzz::ObjectID object, Circle &info);
