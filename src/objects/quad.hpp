#pragma once

#include "builder.hpp"
#include "common.hpp"
#include <physbuzz/physics/dynamics.hpp>
#include <physbuzz/render/transform.hpp>

struct QuadComponent {
    float width = 0.0f;
    float height = 0.0f;
};

struct Quad : public Buildable {
    // physics info
    Physbuzz::RigidBodyComponent body;

    // geometry
    QuadComponent quad;
    Physbuzz::Transform transform;

    // naming
    IdentifiableComponent identifier = {
        .name = "Quad",
        .hidden = false,
    };

    // rendering
    ResourceComponent resources;

    bool hasPhysics = false;
};

template <>
Physbuzz::ObjectID ObjectBuilder::create(Physbuzz::Scene &scene, Physbuzz::ObjectID object, Quad &info);
