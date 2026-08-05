#pragma once

#include "builder.hpp"
#include "common.hpp"
#include <physbuzz/graphics/model.hpp>
#include <physbuzz/math/transform.hpp>

struct ModelComponent {
    std::filesystem::path path;
    bool flipUVs = false;
};

struct Model {
    // geometry
    ModelComponent model;
    Physbuzz::Transform transform;

    // naming
    IdentifiableComponent identifier = {
        .name = "Generic",
        .hidden = false,
    };
};

template <>
struct IsBuildable<Model> : std::true_type {};

template <>
Physbuzz::ObjectID ObjectBuilder::create(Physbuzz::Scene &scene, Physbuzz::ObjectID object, Model &info);
