#include "builder.hpp"

#include <physbuzz/render/cubemap.hpp>
#include <physbuzz/render/model.hpp>
#include <physbuzz/render/texture.hpp>

void ResourceBuilder::buildTextures() {
    Physbuzz::ResourceRegistry<Physbuzz::Texture2D>::insert(
        "default/diffuse",
        {{
            .image = {.file = {.path = "resources/textures/default/diffuse.png"}},
            .type = Physbuzz::TextureType::Diffuse,
        }});

    Physbuzz::ResourceRegistry<Physbuzz::Texture2D>::insert(
        "default/specular",
        {{
            .image = {.file = {.path = "resources/textures/default/specular.png"}},
            .type = Physbuzz::TextureType::Specular,
        }});

    Physbuzz::ResourceRegistry<Physbuzz::Texture2D>::insert(
        "floor",
        {{
            .image = {.file = {.path = "resources/textures/floor.png"}},
            .type = Physbuzz::TextureType::Diffuse,
        }});

    Physbuzz::ResourceRegistry<Physbuzz::Texture2D>::insert(
        "crate/diffuse",
        {{
            .image = {.file = {.path = "resources/textures/crate/diffuse.png"}},
            .type = Physbuzz::TextureType::Diffuse,
        }});

    Physbuzz::ResourceRegistry<Physbuzz::Texture2D>::insert(
        "crate/specular",
        {{
            .image = {.file = {.path = "resources/textures/crate/specular.png"}},
            .type = Physbuzz::TextureType::Specular,
        }});
}

void ResourceBuilder::destroyTextures() {
    Physbuzz::ResourceRegistry<Physbuzz::Texture2D>::erase("default/diffuse");
    Physbuzz::ResourceRegistry<Physbuzz::Texture2D>::erase("default/specular");
    Physbuzz::ResourceRegistry<Physbuzz::Texture2D>::erase("crate/diffuse");
    Physbuzz::ResourceRegistry<Physbuzz::Texture2D>::erase("crate/specular");
    Physbuzz::ResourceRegistry<Physbuzz::Texture2D>::erase("floor");
}

void ResourceBuilder::buildCubemaps() {
    Physbuzz::ResourceRegistry<Physbuzz::Cubemap>::insert(
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
    Physbuzz::ResourceRegistry<Physbuzz::Cubemap>::erase("skybox");
}
