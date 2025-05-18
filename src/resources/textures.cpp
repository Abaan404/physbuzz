#include "builder.hpp"

#include <physbuzz/render/cubemap.hpp>
#include <physbuzz/render/texture.hpp>
#include <physbuzz/resources/manager.hpp>

void ResourceBuilder::buildTextures() {
    Physbuzz::ResourceRegistry<Physbuzz::Texture2DResource>::insert(
        "default/diffuse",
        {{
            .image = {.file = {.path = "resources/textures/default/diffuse.png"}},
        }});

    Physbuzz::ResourceRegistry<Physbuzz::Texture2DResource>::insert(
        "default/specular",
        {{
            .image = {.file = {.path = "resources/textures/default/specular.png"}},
        }});

    Physbuzz::ResourceRegistry<Physbuzz::Texture2DResource>::insert(
        "wall",
        {{
            .image = {.file = {.path = "resources/textures/wall.jpg"}},
        }});

    Physbuzz::ResourceRegistry<Physbuzz::Texture2DResource>::insert(
        "crate/diffuse",
        {{
            .image = {.file = {.path = "resources/textures/crate/diffuse.png"}},
        }});

    Physbuzz::ResourceRegistry<Physbuzz::Texture2DResource>::insert(
        "crate/specular",
        {{
            .image = {.file = {.path = "resources/textures/crate/specular.png"}},
        }});
}

void ResourceBuilder::destroyTextures() {
    Physbuzz::ResourceRegistry<Physbuzz::Texture2DResource>::erase("default/diffuse");
    Physbuzz::ResourceRegistry<Physbuzz::Texture2DResource>::erase("default/specular");
    Physbuzz::ResourceRegistry<Physbuzz::Texture2DResource>::erase("crate/diffuse");
    Physbuzz::ResourceRegistry<Physbuzz::Texture2DResource>::erase("crate/specular");
    Physbuzz::ResourceRegistry<Physbuzz::Texture2DResource>::erase("wall");
}

void ResourceBuilder::buildCubemaps() {
    Physbuzz::ResourceRegistry<Physbuzz::CubemapResource>::insert(
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

void ResourceBuilder::destroyCubemaps() {
    Physbuzz::ResourceRegistry<Physbuzz::CubemapResource>::erase("skybox");
}
