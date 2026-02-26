#pragma once

#include "builder.hpp"
#include "common.hpp"
#include <physbuzz/render/components/camera.hpp>
#include <physbuzz/render/components/lights.hpp>

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
    Physbuzz::SpotLightComponent flashlight;

    // naming
    IdentifiableComponent identifier = {
        .name = "Player",
        .hidden = false,
    };
};

template <>
struct IsBuildable<Player> : std::true_type {};

template <>
Physbuzz::ObjectID ObjectBuilder::create(Physbuzz::Scene &scene, Physbuzz::ObjectID object, Player &info);
