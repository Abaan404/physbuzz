#pragma once

#include "builder.hpp"
#include "common.hpp"
#include <physbuzz/render/transform.hpp>

struct LineComponent {
    float length = 0.0f;
    float thickness = 0.0f;
};

struct Line {
    // geometry
    LineComponent line;
    Physbuzz::TransformComponent transform;

    // naming
    IdentifiableComponent identifier = {
        .name = "Line",
        .hidden = false,
    };

    // rendering
    ResourceComponent resources;
};

template <>
Physbuzz::ObjectID ObjectBuilder::create(Physbuzz::ObjectID object, Line &info);
