#pragma once

#include "builder.hpp"
#include "common.hpp"
#include <physbuzz/render/transform.hpp>

struct CubeComponent {
    float width = 0.0f;
    float height = 0.0f;
    float length = 0.0f;
};

struct Cube : public Buildable {
    // geometry
    CubeComponent cube;
    Physbuzz::Transform transform;

    // naming
    IdentifiableComponent identifier = {
        .name = "Cube",
        .hidden = false,
    };

    // rendering
    ResourceComponent resources;

    bool hasPhysics = false;
};

template <>
Physbuzz::ObjectID ObjectBuilder::create(Physbuzz::Scene &scene, Physbuzz::ObjectID object, Cube &info);
