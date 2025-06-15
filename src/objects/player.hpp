#pragma once

#include "builder.hpp"
#include "common.hpp"
#include <physbuzz/render/camera.hpp>
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

    // naming
    IdentifiableComponent identifier = {
        .name = "Player",
        .hidden = false,
    };
};

template <>
Physbuzz::ObjectID ObjectBuilder::create(Physbuzz::ObjectID object, Player &info);
