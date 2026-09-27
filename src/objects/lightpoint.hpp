#pragma once

#include "builder.hpp"
#include "common.hpp"
#include <physbuzz/render/components/lights.hpp>

struct LightPoint {
    // geometry
    Physbuzz::Transform transform;

    // naming
    IdentifiableComponent identifier = {
        .name = "LightPoint",
        .hidden = false,
    };

    // rendering
    ResourceList resources;
    Physbuzz::PointLightComponent pointLight;
};

template <>
struct IsBuildable<LightPoint> : std::true_type {};

template <>
Physbuzz::ObjectID ObjectBuilder::create(Physbuzz::Scene &scene, Physbuzz::ObjectID object, LightPoint &info);
