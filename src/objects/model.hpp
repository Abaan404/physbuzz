#pragma once

#include "builder.hpp"
#include "common.hpp"
#include <physbuzz/render/model.hpp>
#include <physbuzz/render/transform.hpp>

struct ModelComponent {
    Physbuzz::Resource<Physbuzz::Model> resource;
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

    // rendering
    ResourceComponent resources;
};

template <>
struct IsBuildable<Model> : std::true_type {};

template <>
Physbuzz::ObjectID ObjectBuilder::create(Physbuzz::Scene &scene, Physbuzz::ObjectID object, Model &info);
