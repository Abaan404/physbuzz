#pragma once

#include "builder.hpp"
#include "common.hpp"

struct WallComponent {
    glm::vec3 position;

    float width = 0.0f;
    float height = 0.0f;
    float thickness = 0.0f;

    // wall ids
    Physbuzz::ObjectID left;
    Physbuzz::ObjectID right;
    Physbuzz::ObjectID up;
    Physbuzz::ObjectID down;
};

struct Wall : public Buildable {
    // geometry
    WallComponent wall;

    // naming
    IdentifiableComponent identifier = {
        .name = "Wall",
        .hidden = false,
    };

    bool isCollidable = false;
};

template <>
Physbuzz::ObjectID ObjectBuilder::create(Physbuzz::Scene &scene, Physbuzz::ObjectID object, Wall &info);
