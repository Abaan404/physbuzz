#pragma once

#include "builder.hpp"
#include "common.hpp"
#include <physbuzz/math/transform.hpp>
#include <physbuzz/physics/components/rigidbody.hpp>

struct Shape {
    // geometry
    Physbuzz::Transform transform;
    Physbuzz::RigidBodyComponent body;

    // naming
    IdentifiableComponent identifier = {
        .name = "Cuboid",
        .hidden = false,
    };

    // rendering
    ResourceList resources;

    bool hasPhysics = false;
    bool hasGravity = false;
};

template <>
struct IsBuildable<Shape> : std::true_type {};

template <>
Physbuzz::ObjectID ObjectBuilder::create(Physbuzz::Scene &scene, Physbuzz::ObjectID object, Shape &info);
