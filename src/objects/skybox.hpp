#pragma once

#include "builder.hpp"
#include "common.hpp"
#include "shader/default.hpp"
#include <physbuzz/physics/dynamics.hpp>
#include <physbuzz/render/transform.hpp>

struct SkyboxComponent {
    std::string cubemap;
};

struct Skybox {
    // geometry
    SkyboxComponent skybox;
    Physbuzz::TransformComponent transform;

    // rendering
    ShaderComponent shader = s_DefaultShader;
};

template <>
Physbuzz::ObjectID ObjectBuilder::create(Physbuzz::ObjectID object, Skybox &info);
