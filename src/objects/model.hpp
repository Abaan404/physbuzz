#pragma once

#include "common.hpp"
#include "shader/default.hpp"
#include <physbuzz/physics/dynamics.hpp>
#include <physbuzz/render/transform.hpp>

struct ModelComponent {
    std::string id;
};

struct Model {
    // physics info
    Physbuzz::RigidBodyComponent body;

    // geometry
    ModelComponent model;
    Physbuzz::TransformComponent transform;

    // naming
    IdentifiableComponent identifier = {
        .name = "Generic",
        .hidden = false,
    };

    // rendering
    ShaderComponent shader = s_DefaultShader;

    bool hasPhysics = false;
};

template <>
Physbuzz::ObjectID ObjectBuilder::create(Physbuzz::ObjectID object, Model &info);
