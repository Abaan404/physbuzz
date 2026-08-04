#include "loader.hpp"

#include <physbuzz/graphics/descriptors/texture.hpp>
#include <physbuzz/graphics/material.hpp>
#include <physbuzz/graphics/model.hpp>
#include <physbuzz/resources/registry.hpp>

void ResourceLoader::loadCubemap(const Physbuzz::ResourceID &resourceId, const Physbuzz::ImageFile::Info cubemap, Physbuzz::TransferBatch &batch) {
    Physbuzz::ImageFile cubemapFile = cubemap;
    cubemapFile.readMeta();

    Physbuzz::ResourceRegistry<Physbuzz::Texture>::insert(
        resourceId,
        {{
            .type = Physbuzz::Texture::Type::Cube,
            .sampler = {{Physbuzz::Sampler::Type::Linear}},
        }},
        glm::uvec3{cubemapFile.getData().meta.resolution, 1});

    Physbuzz::Resource<Physbuzz::Texture>{resourceId}->write(cubemap, batch);
}

void ResourceLoader::loadTexture(const Physbuzz::ResourceID &resourceId, const Physbuzz::ImageFile::Info image, Physbuzz::TransferBatch &batch) {
    Physbuzz::ImageFile imageFile = image;
    imageFile.readMeta();

    Physbuzz::ResourceRegistry<Physbuzz::Texture>::insert(
        resourceId,
        {{
            .type = Physbuzz::Texture::Type::Dim2D,
            .mipLevels = Physbuzz::Image::RemainingMipLevels,
            .sampler = {{Physbuzz::Sampler::Type::Linear}},
        }},
        glm::uvec3{imageFile.getData().meta.resolution, 1});

    Physbuzz::Resource<Physbuzz::Texture>{resourceId}->write(image, batch);
}
