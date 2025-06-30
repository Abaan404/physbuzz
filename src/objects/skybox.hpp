#pragma once

#include "builder.hpp"
#include "common.hpp"
#include <physbuzz/render/cubemap.hpp>
#include <physbuzz/render/transform.hpp>

struct SkyboxComponent {
    Physbuzz::Resource<Physbuzz::Cubemap> cubemap = {"skybox"};
};

struct Skybox : public Buildable {
    // geometry
    SkyboxComponent skybox;
    Physbuzz::Transform transform;

    // rendering
    ResourceComponent resources = {
        .pipeline = {"skybox"},
    };
};

template <>
Physbuzz::ObjectID ObjectBuilder::create(Physbuzz::Scene &scene, Physbuzz::ObjectID object, Skybox &info);
