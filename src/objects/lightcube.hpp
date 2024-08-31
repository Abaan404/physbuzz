#pragma once

#include "cube.hpp"
#include <physbuzz/render/lighting.hpp>

struct LightCube {
    Cube cube;

    // position WILL be overriden
    Physbuzz::PointLightComponent pointLight;
};

template <>
Physbuzz::ObjectID ObjectBuilder::create(Physbuzz::ObjectID object, LightCube &info);
