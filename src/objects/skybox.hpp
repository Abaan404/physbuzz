#pragma once

#include "builder.hpp"
#include "common.hpp"
#include <physbuzz/render/cubemap.hpp>
#include <physbuzz/physics/dynamics.hpp>
#include <physbuzz/render/transform.hpp>

struct SkyboxComponent {
    Physbuzz::ResourceHandle<Physbuzz::CubemapResource> cubemap = {"skybox"};
};

struct Skybox {
    // geometry
    SkyboxComponent skybox;
    Physbuzz::TransformComponent transform;

    // rendering
    ResourceComponent resources;
};

template <>
Physbuzz::ObjectID ObjectBuilder::create(Physbuzz::ObjectID object, Skybox &info);
