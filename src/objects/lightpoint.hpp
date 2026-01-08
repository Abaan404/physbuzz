#pragma once

#include "circle.hpp"
#include <physbuzz/render/lighting.hpp>

struct LightPoint {
    // geometry
    RadialComponent sphere;
    Physbuzz::Transform transform;

    // naming
    IdentifiableComponent identifier = {
        .name = "LightPoint",
        .hidden = false,
    };

    // rendering
    ResourceComponent resources;

    bool hasPhysics = false;

    Physbuzz::PointLightComponent pointLight;
};

template <>
struct IsBuildable<LightPoint> : std::true_type {};

template <>
Physbuzz::ObjectID ObjectBuilder::create(Physbuzz::Scene &scene, Physbuzz::ObjectID object, LightPoint &info);
