#pragma once

#include "builder.hpp"
#include "common.hpp"
#include <physbuzz/math/transform.hpp>
#include <physbuzz/physics/components/rigidbody.hpp>

struct Shape {
    // geometry
    Physbuzz::Transform transform;
    Physbuzz::RigidBodyComponent::Info body;

    // naming
    IdentifiableComponent identifier = {
        .name = "Cuboid",
        .hidden = false,
    };

    // rendering
    ResourceComponent resources;

    bool hasPhysics = false;
    bool hasShadow = false;
};

template <>
struct IsBuildable<Shape> : std::true_type {};

template <>
Physbuzz::ObjectID ObjectBuilder::create(Physbuzz::Scene &scene, Physbuzz::ObjectID object, Shape &info);
