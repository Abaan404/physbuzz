#pragma once

#include "common.hpp"
#include <physbuzz/dynamics.hpp>

struct WallComponent {
    float width = 0.0f;
    float height = 0.0f;
    float thickness = 0.0f;

    // wall ids
    Physbuzz::ObjectID left;
    Physbuzz::ObjectID right;
    Physbuzz::ObjectID up;
    Physbuzz::ObjectID down;
};

struct Wall {
    // physics info
    Physbuzz::TransformableComponent transform;

    // geometry
    WallComponent wall;

    // naming
    IdentifiableComponent identifier = {
        .name = "Wall",
        .hidden = false,
    };

    bool isCollidable = false;
    bool isRenderable = false;
};

template <>
Physbuzz::ObjectID ObjectBuilder::create(Physbuzz::Object &object, Wall &info);
