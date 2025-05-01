#pragma once

#include "builder.hpp"
#include "common.hpp"
#include <physbuzz/physics/dynamics.hpp>
#include <physbuzz/render/model.hpp>

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
    std::string pipeline = "quad";
    TextureResources textures = {
        .texture2D = {
            {Physbuzz::TextureType::Diffuse, {"default/diffuse"}},
            {Physbuzz::TextureType::Specular, {"default/specular"}},
        },
    };

    bool hasPhysics = false;
};

template <>
Physbuzz::ObjectID ObjectBuilder::create(Physbuzz::ObjectID object, Quad &info);
