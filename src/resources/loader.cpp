#include "loader.hpp"

#include <physbuzz/graphics/descriptors/texture.hpp>
#include <physbuzz/graphics/material.hpp>
#include <physbuzz/graphics/model.hpp>
#include <physbuzz/physics/defines.hpp>
#include <physbuzz/render/shadow.hpp>
#include <physbuzz/resources/registry.hpp>

std::optional<Physbuzz::Resource<Physbuzz::Texture>> ResourceLoader::loadCubemap(const std::string &name, const Physbuzz::ImageFile::Info &cubemap, Physbuzz::TransferBatch &batch) {
    Physbuzz::ImageFile cubemapFile = cubemap;
    cubemapFile.readMeta();

    Physbuzz::Texture::Info info = {
        .type = Physbuzz::Texture::Type::Cube,
        .sampler = {{Physbuzz::Sampler::Type::Linear}},
    };

    auto texture = Physbuzz::ResourceRegistry<Physbuzz::Texture>::insert(
        name,
        Physbuzz::Texture{info},
        glm::uvec3{cubemapFile.getData().meta.resolution, 1});

    if (!texture) {
        return std::nullopt;
    }

    if (!(*texture)->write(cubemap, batch)) {
        return std::nullopt;
    }

    return texture;
}

std::optional<Physbuzz::Resource<Physbuzz::Texture>> ResourceLoader::loadTexture(const std::string &name, const Physbuzz::ImageFile::Info &image, Physbuzz::TransferBatch &batch) {
    Physbuzz::ImageFile imageFile = image;
    imageFile.readMeta();

    Physbuzz::Texture::Info info = {
        .type = Physbuzz::Texture::Type::Dim2D,
        .mipLevels = Physbuzz::Image::RemainingMipLevels,
        .sampler = {{Physbuzz::Sampler::Type::Linear}},
    };

    auto texture = Physbuzz::ResourceRegistry<Physbuzz::Texture>::insert(
        name,
        Physbuzz::Texture{info},
        glm::uvec3{imageFile.getData().meta.resolution, 1}
    );

    if (!texture) {
        return std::nullopt;
    }

    if (!(*texture)->write(image, batch)) {
        return std::nullopt;
    }

    return texture;
}

std::optional<Physbuzz::Resource<Physbuzz::Mesh>> ResourceLoader::loadMesh(const std::string &name, const Physbuzz::Geometry<Physbuzz::Model::Vertex> &geometry, Physbuzz::TransferBatch &batch) {
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

    auto mesh = Physbuzz::ResourceRegistry<Physbuzz::Mesh>::insert(name, Physbuzz::Mesh{info});

    if (!mesh) {
        return std::nullopt;
    }

    if (!(*mesh)->write(Physbuzz::Geometry(geometry), batch)) {
        return std::nullopt;
    }

    return mesh;
}

std::optional<Physbuzz::Resource<Physbuzz::Mesh>> ResourceLoader::loadShadowMesh(const std::string &name, const Physbuzz::Geometry<Physbuzz::Model::Vertex> &geometry, Physbuzz::TransferBatch &batch) {
    Physbuzz::AABB::Info aabb;

    for (const auto &vertex : geometry.vertices) {
        aabb.min = glm::min(vertex.position, aabb.min);
        aabb.max = glm::max(vertex.position, aabb.max);
    }

    Physbuzz::Geometry<Physbuzz::ShadowRenderer::Vertex> shadowGeometry = {
        .indices = geometry.indices,
    };

    shadowGeometry.vertices.reserve(geometry.vertices.size());
    for (const auto &vertex : geometry.vertices) {
        shadowGeometry.vertices.emplace_back<Physbuzz::ShadowRenderer::Vertex>({
            .position = vertex.position,
        });
    }

    Physbuzz::Mesh::Info info = {
        .description = &Physbuzz::ShadowRenderer::Vertex::Description,
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

    auto mesh = Physbuzz::ResourceRegistry<Physbuzz::Mesh>::insert(name, Physbuzz::Mesh{info});
    if (!mesh) {
        return std::nullopt;
    }

    if (!(*mesh)->write(std::move(shadowGeometry), batch)) {
        return std::nullopt;
    }

    return mesh;
}

std::optional<Physbuzz::Resource<Physbuzz::PhysicsMesh>> ResourceLoader::loadPhysicsMesh(const std::string &name, const Physbuzz::Geometry<Physbuzz::Model::Vertex> &geometry, std::function<glm::vec3(const glm::vec3 &)> support) {
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
        .supportPoint = support,
    };

    physicsMesh.geometry.vertices.reserve(geometry.vertices.size());
    for (const auto &vertex : geometry.vertices) {
        physicsMesh.geometry.vertices.emplace_back<Physbuzz::PhysicsMesh::Vertex>({
            .position = vertex.position,
            .normal = vertex.normal,
        });
    }

    return Physbuzz::ResourceRegistry<Physbuzz::PhysicsMesh>::insert(name, std::move(physicsMesh));
}
