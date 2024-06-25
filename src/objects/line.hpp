#pragma once

#include <physbuzz/dynamics.hpp>
#include "objects.hpp"

struct LineComponent {
    float length = 0.0f;
    float thickness = 0.0f;
};

struct LineInfo {
    // geometry
    Physbuzz::TransformableComponent transform;
    LineComponent line;

    // naming
    IdentifiableComponent identifier = {
        .type = ObjectType::Line,
        .name = "Line",
    };

    bool isRenderable = false;
};

template <>
Physbuzz::ObjectID ObjectBuilder<LineInfo>::build(Physbuzz::Object &object, LineInfo &info);
