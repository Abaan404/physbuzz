#pragma once

#include "builder.hpp"
#include "common.hpp"
#include <physbuzz/physics/dynamics.hpp>
#include <physbuzz/render/transform.hpp>

struct SkyboxComponent {
    std::string cubemap = "skybox";
};

struct Skybox {
    // geometry
    SkyboxComponent skybox;
    Physbuzz::TransformComponent transform;

    // rendering
    ResourceComponent resources = {
        .pipeline = "skybox",
    };
};

template <>
Physbuzz::ObjectID ObjectBuilder::create(Physbuzz::ObjectID object, Skybox &info);
