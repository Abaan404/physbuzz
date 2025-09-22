#include "builder.hpp"

#include <physbuzz/render/cubemap.hpp>
#include <physbuzz/render/layouts/texture.hpp>
#include <physbuzz/render/model.hpp>

void ResourceBuilder::buildTextures() {
    // Physbuzz::ResourceRegistry<Physbuzz::Texture>::insert(
    //     "default/diffuse",
    //     {{
    //         .image = {.file = {.path = "resources/textures/default/diffuse.png"}},
    //         .type = Physbuzz::TextureType::Diffuse,
    //     }});
    //
    // Physbuzz::ResourceRegistry<Physbuzz::Texture>::insert(
    //     "default/specular",
    //     {{
    //         .image = {.file = {.path = "resources/textures/default/specular.png"}},
    //         .type = Physbuzz::TextureType::Specular,
    //     }});
    //
    // Physbuzz::ResourceRegistry<Physbuzz::Texture>::insert(
    //     "floor",
    //     {{
    //         .image = {.file = {.path = "resources/textures/floor.png"}},
    //         .type = Physbuzz::TextureType::Diffuse,
    //     }});
    //
    // Physbuzz::ResourceRegistry<Physbuzz::Texture>::insert(
    //     "crate/diffuse",
    //     {{
    //         .image = {.file = {.path = "resources/textures/crate/diffuse.png"}},
    //         .type = Physbuzz::TextureType::Diffuse,
    //     }});
    //
    // Physbuzz::ResourceRegistry<Physbuzz::Texture>::insert(
    //     "crate/specular",
    //     {{
    //         .image = {.file = {.path = "resources/textures/crate/specular.png"}},
    //         .type = Physbuzz::TextureType::Specular,
    //     }});
}

void ResourceBuilder::destroyTextures() {
    // Physbuzz::ResourceRegistry<Physbuzz::Texture>::erase("default/diffuse");
    // Physbuzz::ResourceRegistry<Physbuzz::Texture>::erase("default/specular");
    // Physbuzz::ResourceRegistry<Physbuzz::Texture>::erase("crate/diffuse");
    // Physbuzz::ResourceRegistry<Physbuzz::Texture>::erase("crate/specular");
    // Physbuzz::ResourceRegistry<Physbuzz::Texture>::erase("floor");
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
