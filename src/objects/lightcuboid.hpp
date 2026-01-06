#pragma once

#include "cuboid.hpp"
#include <physbuzz/render/lighting.hpp>

struct LightCuboid {
    // geometry
    CuboidComponent cuboid;
    Physbuzz::Transform transform;

    // naming
    IdentifiableComponent identifier = {
        .name = "LightCuboid",
        .hidden = false,
    };

    // rendering
    ResourceComponent resources;

    bool hasPhysics = false;

    Physbuzz::PointLightComponent pointLight;
};

template <>
struct IsBuildable<LightCuboid> : std::true_type {};

template <>
Physbuzz::ObjectID ObjectBuilder::create(Physbuzz::Scene &scene, Physbuzz::ObjectID object, LightCuboid &info);
