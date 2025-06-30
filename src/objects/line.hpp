#pragma once

#include "builder.hpp"
#include "common.hpp"
#include <physbuzz/ecs/scene.hpp>
#include <physbuzz/render/transform.hpp>

struct LineComponent {
    float length = 0.0f;
    float thickness = 0.0f;
};

struct Line {
    // geometry
    LineComponent line;
    Physbuzz::Transform transform;

    // naming
    IdentifiableComponent identifier = {
        .name = "Line",
        .hidden = false,
    };

    // rendering
    ResourceComponent resources;
};

template <>
struct IsBuildable<Line> : std::true_type {};

template <>
Physbuzz::ObjectID ObjectBuilder::create(Physbuzz::Scene &scene, Physbuzz::ObjectID object, Line &info);
