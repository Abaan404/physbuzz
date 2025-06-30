#pragma once

#include "builder.hpp"
#include "common.hpp"
#include <physbuzz/render/lighting.hpp>

struct LightDirectional {
    Physbuzz::DirectionalLightComponent directionalLight;

    IdentifiableComponent identifier = {
        .name = "LightDirectional",
        .hidden = false,
    };
};

template <>
struct IsBuildable<LightDirectional> : std::true_type {};

template <>
Physbuzz::ObjectID ObjectBuilder::create(Physbuzz::Scene &scene, Physbuzz::ObjectID object, LightDirectional &info);
