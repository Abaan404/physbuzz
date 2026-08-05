#pragma once

#include "builder.hpp"
#include "common.hpp"
#include <physbuzz/math/transform.hpp>
#include <physbuzz/physics/dynamics.hpp>

struct MaterialSphere {
    // geometry
    RadialComponent sphere;
    Physbuzz::Transform transform;

    // resources
    ResourceComponent resources;
};

template <>
struct IsBuildable<MaterialSphere> : std::true_type {};

template <>
Physbuzz::ObjectID ObjectBuilder::create(Physbuzz::Scene &scene, Physbuzz::ObjectID object, MaterialSphere &info);
