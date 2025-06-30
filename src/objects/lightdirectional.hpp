#pragma once

#include "builder.hpp"
#include "common.hpp"
#include <physbuzz/render/lighting.hpp>

struct LightDirectional : public Buildable {
    Physbuzz::DirectionalLightComponent directionalLight;

    IdentifiableComponent identifier = {
        .name = "LightDirectional",
        .hidden = false,
    };
};

template <>
Physbuzz::ObjectID ObjectBuilder::create(Physbuzz::Scene &scene, Physbuzz::ObjectID object, LightDirectional &info);
