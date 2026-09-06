#pragma once

#include "builder.hpp"
#include "common.hpp"
#include <physbuzz/math/transform.hpp>
#include <physbuzz/physics/dynamics.hpp>

struct MaterialShape {
    // geometry
    Physbuzz::Transform transform;

    // resources
    ResourceComponent resources;
};

template <>
struct IsBuildable<MaterialShape> : std::true_type {};

template <>
Physbuzz::ObjectID ObjectBuilder::create(Physbuzz::Scene &scene, Physbuzz::ObjectID object, MaterialShape &info);
