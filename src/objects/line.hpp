#pragma once

#include "builder.hpp"
#include "common.hpp"

struct LineComponent {
    float length = 0.0f;
    float thickness = 0.0f;
};

struct Line {
    // geometry
    Physbuzz::Model model;
    LineComponent line;

    // naming
    IdentifiableComponent identifier = {
        .name = "Line",
    };

    ResourceIdentifierComponent resources = {
        .pipeline = "cube",
    };

    Physbuzz::Material material = {
        .diffuse = "missing",
        .specular = "missing_specular",
    };

    bool isRenderable = false;
};

template <>
Physbuzz::ObjectID ObjectBuilder::create(Physbuzz::ObjectID object, Line &info);
