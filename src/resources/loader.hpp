#pragma once

#include <physbuzz/graphics/descriptors/texture.hpp>
#include <physbuzz/graphics/model.hpp>
#include <physbuzz/graphics/transfer.hpp>
#include <physbuzz/io/image.hpp>

class ResourceLoader {
  public:
    static void loadCubemap(const Physbuzz::ResourceID &resourceId, const Physbuzz::ImageFile::Info cubemap, Physbuzz::TransferBatch &batch);
    static void loadTexture(const Physbuzz::ResourceID &resourceId, const Physbuzz::ImageFile::Info image, Physbuzz::TransferBatch &batch);
};
