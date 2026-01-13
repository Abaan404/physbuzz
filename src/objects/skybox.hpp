#pragma once

#include "builder.hpp"
#include <physbuzz/render/cubemap.hpp>
#include <physbuzz/render/transform.hpp>
#include <physbuzz/resources/resource.hpp>

struct SkyboxComponent {
    Physbuzz::Resource<Physbuzz::Cubemap> cubemap = {"skybox"};
};

struct Skybox {
    // geometry
    SkyboxComponent skybox;
    Physbuzz::Transform transform;
};

template <>
struct IsBuildable<Skybox> : std::true_type {};

template <>
Physbuzz::ObjectID ObjectBuilder::create(Physbuzz::Scene &scene, Physbuzz::ObjectID object, Skybox &info);
