#pragma once

#include "builder.hpp"
#include "common.hpp"
#include "shader/default.hpp"
#include <physbuzz/physics/dynamics.hpp>
#include <physbuzz/render/transform.hpp>

struct QuadComponent {
    float width = 0.0f;
    float height = 0.0f;
};

struct Quad {
    // physics info
    Physbuzz::RigidBodyComponent body;

    // geometry
    QuadComponent quad;
    Physbuzz::TransformComponent transform;

    // naming
    IdentifiableComponent identifier = {
        .name = "Quad",
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
Physbuzz::ObjectID ObjectBuilder::create(Physbuzz::ObjectID object, Quad &info);
