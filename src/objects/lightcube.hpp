#pragma once

#include "cube.hpp"
#include <physbuzz/render/lighting.hpp>

struct LightCube {
    Cube cube;

    Physbuzz::PointLightComponent pointLight;
};

template <>
struct IsBuildable<LightCube> : std::true_type {};

template <>
Physbuzz::ObjectID ObjectBuilder::create(Physbuzz::Scene &scene, Physbuzz::ObjectID object, LightCube &info);
