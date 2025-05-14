#pragma once

#include "shaders/circle.hpp"
#include "shaders/cube.hpp"
#include "shaders/default.hpp"
#include "shaders/quad.hpp"
#include "shaders/skybox.hpp"
#include "uniforms/time.hpp"
#include "uniforms/window.hpp"
#include <physbuzz/render/cubemap.hpp>

class ResourceBuilder {
  public:
    inline void buildTextures() {
        Physbuzz::ResourceRegistry::insert<Physbuzz::Texture2DResource>(
            "default/diffuse",
            {{
                .image = {.file = {.path = "resources/textures/default/diffuse.png"}},
            }});

        Physbuzz::ResourceRegistry::insert<Physbuzz::Texture2DResource>(
            "default/specular",
            {{
                .image = {.file = {.path = "resources/textures/default/specular.png"}},
            }});

        Physbuzz::ResourceRegistry::insert<Physbuzz::Texture2DResource>(
            "wall",
            {{
                .image = {.file = {.path = "resources/textures/wall.jpg"}},
            }});

        Physbuzz::ResourceRegistry::insert<Physbuzz::Texture2DResource>(
            "crate/diffuse",
            {{
                .image = {.file = {.path = "resources/textures/crate/diffuse.png"}},
            }});

        Physbuzz::ResourceRegistry::insert<Physbuzz::Texture2DResource>(
            "crate/specular",
            {{
                .image = {.file = {.path = "resources/textures/crate/specular.png"}},
            }});
    }

    inline void buildModels() {
        Physbuzz::ResourceRegistry::insert<Physbuzz::ModelResource>(
            "backpack",
            {{
                "resources/models/backpack/backpack.obj",
            }});
    }

    inline void buildCubemaps() {
        Physbuzz::ResourceRegistry::insert<Physbuzz::CubemapResource>(
            "skybox",
            {{
                .right = {.file = {.path = "resources/textures/skybox/right.jpg"}},
                .left = {.file = {.path = "resources/textures/skybox/left.jpg"}},
                .top = {.file = {.path = "resources/textures/skybox/top.jpg"}},
                .bottom = {.file = {.path = "resources/textures/skybox/bottom.jpg"}},
                .back = {.file = {.path = "resources/textures/skybox/back.jpg"}},
                .front = {.file = {.path = "resources/textures/skybox/front.jpg"}},
            }});
    }

    inline void buildShaders() {
        Physbuzz::ResourceRegistry::insert<Physbuzz::ShaderPipelineResource>("default", std::move(shaderDefault));
        Physbuzz::ResourceRegistry::insert<Physbuzz::ShaderPipelineResource>("circle", std::move(shaderCircle));
        Physbuzz::ResourceRegistry::insert<Physbuzz::ShaderPipelineResource>("quad", std::move(shaderQuad));
        Physbuzz::ResourceRegistry::insert<Physbuzz::ShaderPipelineResource>("cube", std::move(shaderCube));
        Physbuzz::ResourceRegistry::insert<Physbuzz::ShaderPipelineResource>("skybox", std::move(shaderSkybox));
    }

    inline void buildUniforms() {
        Physbuzz::ResourceRegistry::insert<Physbuzz::UniformBufferResource<UniformCamera>>("camera", {});
        Physbuzz::ResourceRegistry::insert<Physbuzz::UniformBufferResource<UniformTime>>("time", {});
        Physbuzz::ResourceRegistry::insert<Physbuzz::UniformBufferResource<UniformWindow>>("window", {});
    }
};
