#pragma once

#include "builder.hpp"
#include "common.hpp"
#include <physbuzz/dynamics.hpp>

struct LineComponent {
    float length = 0.0f;
    float thickness = 0.0f;
};

struct Line {
    // geometry
    Physbuzz::TransformableComponent transform;
    LineComponent line;

    // naming
    IdentifiableComponent identifier = {
        .name = "Line",
    };

    ResourceIdentifierComponent resources = {
        .pipeline = "cube",
    };

    bool isRenderable = false;
};

template <>
Physbuzz::ObjectID ObjectBuilder::create(Physbuzz::Object &object, Line &info);
