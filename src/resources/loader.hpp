#pragma once

#include <physbuzz/graphics/descriptors/texture.hpp>
#include <physbuzz/graphics/mesh.hpp>
#include <physbuzz/graphics/model.hpp>
#include <physbuzz/graphics/transfer.hpp>
#include <physbuzz/io/image.hpp>
#include <physbuzz/physics/defines.hpp>

class ResourceLoader {
  public:
    static std::optional<Physbuzz::Resource<Physbuzz::Texture>> loadCubemap(const std::string &name, const Physbuzz::ImageFile::Info &cubemap, Physbuzz::TransferBatch &batch);
    static std::optional<Physbuzz::Resource<Physbuzz::Texture>> loadTexture(const std::string &name, const Physbuzz::ImageFile::Info &image, Physbuzz::TransferBatch &batch);

    static std::optional<Physbuzz::Resource<Physbuzz::Mesh>> loadMesh(const std::string &name, const Physbuzz::Geometry<Physbuzz::Model::Vertex> &geometry, Physbuzz::TransferBatch &batch);
    static std::optional<Physbuzz::Resource<Physbuzz::Mesh>> loadShadowMesh(const std::string &name, const Physbuzz::Geometry<Physbuzz::Model::Vertex> &geometry, Physbuzz::TransferBatch &batch);
    static std::optional<Physbuzz::Resource<Physbuzz::PhysicsMesh>> loadPhysicsMesh(const std::string &name, const Physbuzz::Geometry<Physbuzz::Model::Vertex> &geometry, std::function<glm::vec3(const glm::vec3 &)> support = nullptr);
};
