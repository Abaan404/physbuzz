#include "sphere.hpp"

#include "../graphics/mesh.hpp"
#include "../graphics/model.hpp"
#include <glm/ext/scalar_constants.hpp>

namespace Physbuzz {

namespace Builtin {

bool ModelSphere::build(TransferBatch &batch) {
    if (ResourceRegistry<Mesh>::contains(Resource)) {
        return true;
    }

    constexpr std::uint32_t rings = 32;
    constexpr std::uint32_t sectors = 64;

    Mesh::Info info = {
        .description = &Model::Vertex::Description,
        .vertexCount = (rings + 1) * (sectors + 1),
        .indexCount = rings * sectors * 6,
        .submeshes = {
            {
                .indexCount = rings * sectors * 6,
                .firstIndex = 0,
                .vertexOffset = 0,
            },
        },
    };

    if (!Physbuzz::ResourceRegistry<Physbuzz::Mesh>::insert(Resource, info)) {
        return false;
    }

    std::vector<Model::Vertex> vertices;
    std::vector<Index> indices;

    vertices.reserve(info.vertexCount);
    indices.reserve(info.indexCount);

    for (std::uint32_t ring = 0; ring <= rings; ++ring) {
        float v = static_cast<float>(ring) / rings;
        float theta = v * glm::pi<float>();

        for (std::uint32_t sector = 0; sector <= sectors; ++sector) {
            float u = static_cast<float>(sector) / sectors;
            float phi = u * 2 * glm::pi<float>();

            // source: https://en.wikipedia.org/wiki/Sphere#Parametric
            glm::vec3 position = {
                std::sin(theta) * std::cos(phi),
                std::sin(theta) * std::sin(phi),
                std::cos(theta),
            };

            glm::vec3 normal = glm::normalize(position);

            glm::vec3 tangent = {-std::sin(theta), 0.0f, std::cos(theta)};
            tangent = glm::normalize(tangent - normal * glm::dot(normal, tangent));

            vertices.emplace_back<Physbuzz::Model::Vertex>({
                .position = position,
                .normal = normal,
                .tangent = tangent,
                .texCoord0 = {u, 1.0f - v},
            });
        }
    }

    const std::uint32_t stride = sectors + 1;

    for (std::uint32_t r = 0; r < rings; ++r) {
        for (std::uint32_t s = 0; s < sectors; ++s) {
            std::uint32_t i0 = r * stride + s;
            std::uint32_t i1 = (r + 1) * stride + s;
            std::uint32_t i2 = (r + 1) * stride + (s + 1);
            std::uint32_t i3 = r * stride + (s + 1);

            indices.emplace_back(i0);
            indices.emplace_back(i1);
            indices.emplace_back(i2);

            indices.emplace_back(i0);
            indices.emplace_back(i2);
            indices.emplace_back(i3);
        }
    }

    return Resource->write(std::move(vertices), std::move(indices), batch);
}

} // namespace Builtin

} // namespace Physbuzz
