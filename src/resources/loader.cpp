#include "loader.hpp"

#include <physbuzz/graphics/descriptors/texture.hpp>
#include <physbuzz/graphics/material.hpp>
#include <physbuzz/graphics/model.hpp>
#include <physbuzz/physics/defines.hpp>
#include <physbuzz/render/shadow.hpp>
#include <physbuzz/resources/registry.hpp>

bool ResourceLoader::loadCubemap(const Physbuzz::ResourceID &resourceId, const Physbuzz::ImageFile::Info &cubemap, Physbuzz::TransferBatch &batch) {
    Physbuzz::ImageFile cubemapFile = cubemap;
    cubemapFile.readMeta();

    Physbuzz::Texture::Info info = {
        .type = Physbuzz::Texture::Type::Cube,
        .sampler = {{Physbuzz::Sampler::Type::Linear}},
    };

    if (!Physbuzz::ResourceRegistry<Physbuzz::Texture>::insert(resourceId, info, glm::uvec3{cubemapFile.getData().meta.resolution, 1})) {
        return false;
    }

    if (!Physbuzz::Resource<Physbuzz::Texture>{resourceId}->write(cubemap, batch)) {
        return false;
    }

    return true;
}

bool ResourceLoader::loadTexture(const Physbuzz::ResourceID &resourceId, const Physbuzz::ImageFile::Info &image, Physbuzz::TransferBatch &batch) {
    Physbuzz::ImageFile imageFile = image;
    imageFile.readMeta();

    Physbuzz::Texture::Info info = {
        .type = Physbuzz::Texture::Type::Dim2D,
        .mipLevels = Physbuzz::Image::RemainingMipLevels,
        .sampler = {{Physbuzz::Sampler::Type::Linear}},
    };

    if (!Physbuzz::ResourceRegistry<Physbuzz::Texture>::insert(resourceId, info, glm::uvec3{imageFile.getData().meta.resolution, 1})) {
        return false;
    }

    if (!Physbuzz::Resource<Physbuzz::Texture>{resourceId}->write(image, batch)) {
        return false;
    }

    return true;
}

bool ResourceLoader::loadMesh(const Physbuzz::ResourceID &resourceId, const Physbuzz::Geometry<Physbuzz::Model::Vertex> &geometry, Physbuzz::TransferBatch &batch) {
    Physbuzz::AABB::Info aabb;

    for (const auto &vertex : geometry.vertices) {
        aabb.min = glm::min(vertex.position, aabb.min);
        aabb.max = glm::max(vertex.position, aabb.max);
    }

    Physbuzz::Mesh::Info info = {
        .description = &Physbuzz::Model::Vertex::Description,
        .vertexCount = static_cast<std::uint32_t>(geometry.vertices.size()),
        .indexCount = static_cast<std::uint32_t>(geometry.indices.size()),
        .submeshes = {
            {
                .bounding = {aabb},
                .indexCount = static_cast<std::uint32_t>(geometry.indices.size()),
                .firstIndex = 0,
                .vertexOffset = 0,
            },
        },
    };

    if (!Physbuzz::ResourceRegistry<Physbuzz::Mesh>::insert(resourceId, info)) {
        return false;
    }

    if (!Physbuzz::Resource<Physbuzz::Mesh>{resourceId}->write(Physbuzz::Geometry(geometry), batch)) {
        return false;
    }

    return true;
}

bool ResourceLoader::loadShadowMesh(const Physbuzz::ResourceID &resourceId, const Physbuzz::Geometry<Physbuzz::Model::Vertex> &geometry, Physbuzz::TransferBatch &batch) {
    Physbuzz::AABB::Info aabb;

    for (const auto &vertex : geometry.vertices) {
        aabb.min = glm::min(vertex.position, aabb.min);
        aabb.max = glm::max(vertex.position, aabb.max);
    }

    Physbuzz::Geometry<Physbuzz::Builtin::PipelineShadow::Vertex> shadowGeometry = {
        .indices = geometry.indices,
    };

    shadowGeometry.vertices.reserve(geometry.vertices.size());
    for (const auto &vertex : geometry.vertices) {
        shadowGeometry.vertices.emplace_back<Physbuzz::Builtin::PipelineShadow::Vertex>({
            .position = vertex.position,
        });
    }

    Physbuzz::Mesh::Info info = {
        .description = &Physbuzz::Builtin::PipelineShadow::Vertex::Description,
        .vertexCount = static_cast<std::uint32_t>(shadowGeometry.vertices.size()),
        .indexCount = static_cast<std::uint32_t>(shadowGeometry.indices.size()),
        .submeshes = {
            {
                .bounding = {aabb},
                .indexCount = static_cast<std::uint32_t>(shadowGeometry.indices.size()),
                .firstIndex = 0,
                .vertexOffset = 0,
            },
        },
    };

    if (!Physbuzz::ResourceRegistry<Physbuzz::Mesh>::insert(resourceId, info)) {
        return false;
    }

    if (!Physbuzz::Resource<Physbuzz::Mesh>{resourceId}->write(std::move(shadowGeometry), batch)) {
        return false;
    }

    return true;
}

bool ResourceLoader::loadPhysicsMesh(const Physbuzz::ResourceID &resourceId, const Physbuzz::Geometry<Physbuzz::Model::Vertex> &geometry) {
    Physbuzz::AABB::Info aabb;

    for (const auto &vertex : geometry.vertices) {
        aabb.min = glm::min(vertex.position, aabb.min);
        aabb.max = glm::max(vertex.position, aabb.max);
    }

    Physbuzz::PhysicsMesh physicsMesh = {
        .geometry = {
            .indices = std::move(geometry.indices),
        },
        .bounding = {aabb},
    };

    physicsMesh.geometry.vertices.reserve(geometry.vertices.size());
    for (const auto &vertex : geometry.vertices) {
        physicsMesh.geometry.vertices.emplace_back<Physbuzz::PhysicsMesh::Vertex>({
            .position = vertex.position,
            .normal = vertex.normal,
        });
    }

    if (!Physbuzz::ResourceRegistry<Physbuzz::PhysicsMesh>::insert(resourceId, std::move(physicsMesh))) {
        return false;
    }

    return true;
}
