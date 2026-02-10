#pragma once

#include "builder.hpp"
#include "common.hpp"
#include <physbuzz/graphics/transform.hpp>

struct CuboidComponent {
    float width = 0.0f;
    float breadth = 0.0f;
    float height = 0.0f;
};

struct Cuboid {
    // geometry
    CuboidComponent cuboid;
    Physbuzz::Transform transform;

    // naming
    IdentifiableComponent identifier = {
        .name = "Cuboid",
        .hidden = false,
    };

    // rendering
    ResourceComponent resources;

    bool hasPhysics = false;
};

template <>
struct IsBuildable<Cuboid> : std::true_type {};

template <>
Physbuzz::ObjectID ObjectBuilder::create(Physbuzz::Scene &scene, Physbuzz::ObjectID object, Cuboid &info);
