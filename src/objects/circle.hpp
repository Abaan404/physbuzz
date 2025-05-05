#pragma once

#include "common.hpp"
#include "shader/default.hpp"
#include <physbuzz/physics/dynamics.hpp>
#include <physbuzz/render/model.hpp>

struct CircleComponent {
    float radius = 0.0f;
};

struct Circle {
    // physics info
    Physbuzz::RigidBodyComponent body;

    // geometry
    CircleComponent circle;
    Physbuzz::TransformComponent transform;

    // naming
    IdentifiableComponent identifier = {
        .name = "Circle",
        .hidden = false,
    };

    // rendering
    ShaderComponent shader = s_DefaultShader;
    TextureResources textures = {
        .resource = {
            {Physbuzz::TextureType::Diffuse, {"default/diffuse"}},
            {Physbuzz::TextureType::Specular, {"default/specular"}},
        },
    };

    bool hasPhysics = false;
};

template <>
Physbuzz::ObjectID ObjectBuilder::create(Physbuzz::ObjectID object, Circle &info);
