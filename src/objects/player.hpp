#pragma once

#include "builder.hpp"
#include "common.hpp"
#include <physbuzz/render/camera.hpp>
#include <physbuzz/render/lighting.hpp>
#include <physbuzz/render/transform.hpp>
#include <physbuzz/window/bindings.hpp>

struct PlayerComponent {
    float speed = 5.0f;
    float sensitivity = 0.1f;

    bool captureMouse = false;
};

struct Player {
    // physics info
    Physbuzz::CameraComponent camera;

    // geometry
    PlayerComponent player;
    Physbuzz::SpotLightComponent flashlight = {
        .ambient = {0.2f, 0.2f, 0.2f},
        .diffuse = {0.5f, 0.5f, 0.5f},
        .specular = {1.0f, 1.0f, 1.0f},

        .constant = 1.0f,
        .linear = 0.0009f,
        .quadratic = 0.000032f,

        .cutOff = glm::radians(12.5f),
        .outerCutOff = glm::radians(17.5f),
    };

    // naming
    IdentifiableComponent identifier = {
        .name = "Player",
        .hidden = false,
    };
};

template <>
Physbuzz::ObjectID ObjectBuilder::create(Physbuzz::ObjectID object, Player &info);
