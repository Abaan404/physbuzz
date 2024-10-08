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
    Physbuzz::Model model;
    CircleComponent circle;

    // naming
    IdentifiableComponent identifier = {
        .name = "Circle",
    };

    ResourceIdentifierComponent resources = {
        .pipeline = "circle",
    };

    Physbuzz::Material material = {
        .diffuse = "missing",
        .specular = "missing_specular",
    };

    bool isCollidable = false;
    bool isRenderable = false;
};

template <>
Physbuzz::ObjectID ObjectBuilder::create(Physbuzz::ObjectID object, Circle &info);
