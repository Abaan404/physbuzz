#pragma once

#include <physbuzz/graphics/descriptors/texture.hpp>
#include <physbuzz/graphics/mesh.hpp>
#include <physbuzz/graphics/model.hpp>
#include <physbuzz/graphics/transfer.hpp>
#include <physbuzz/io/image.hpp>

class ResourceLoader {
  public:
    static bool loadCubemap(const Physbuzz::ResourceID &resourceId, const Physbuzz::ImageFile::Info &cubemap, Physbuzz::TransferBatch &batch);
    static bool loadTexture(const Physbuzz::ResourceID &resourceId, const Physbuzz::ImageFile::Info &image, Physbuzz::TransferBatch &batch);
    static bool loadMesh(const Physbuzz::ResourceID &resourceId, Physbuzz::Geometry<Physbuzz::Model::Vertex> &&geometry, Physbuzz::TransferBatch &batch);
    static bool loadPhysicsMesh(const Physbuzz::ResourceID &resourceId, Physbuzz::Geometry<Physbuzz::Model::Vertex> &&geometry);
};
