#pragma once

#include "cube.hpp"
#include <physbuzz/render/lighting.hpp>

struct LightCube : public Buildable {
    Cube cube;

    Physbuzz::PointLightComponent pointLight;
};

template <>
Physbuzz::ObjectID ObjectBuilder::create(Physbuzz::Scene &scene, Physbuzz::ObjectID object, LightCube &info);
