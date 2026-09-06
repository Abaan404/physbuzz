#include "sphere.hpp"

#include <glm/ext/scalar_constants.hpp>

Physbuzz::Geometry<Physbuzz::Model::Vertex> ModelSphere::build() {
    Physbuzz::Geometry<Physbuzz::Model::Vertex> geometry;

    constexpr std::uint32_t rings = 32;
    constexpr std::uint32_t sectors = 64;

    std::uint32_t vertexCount = (rings + 1) * (sectors + 1);
    std::uint32_t indexCount = rings * sectors * 6;

    geometry.vertices.reserve(vertexCount);
    geometry.indices.reserve(indexCount);

    for (std::uint32_t ring = 0; ring <= rings; ++ring) {
        float v = static_cast<float>(ring) / rings;
        float theta = v * glm::pi<float>();

        for (std::uint32_t sector = 0; sector <= sectors; ++sector) {
            float u = static_cast<float>(sector) / sectors;
            float phi = u * 2 * glm::pi<float>();

            // source: https://en.wikipedia.org/wiki/Sphere#Parametric
            glm::vec3 position = glm::normalize(glm::vec3{
                std::sin(theta) * std::cos(phi),
                std::sin(theta) * std::sin(phi),
                std::cos(theta),
            });

            glm::vec3 normal = position;
            glm::vec3 tangent = {-std::sin(phi), std::cos(phi), 0.0f};

            geometry.vertices.emplace_back<Physbuzz::Model::Vertex>({
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

            geometry.indices.emplace_back(i0);
            geometry.indices.emplace_back(i1);
            geometry.indices.emplace_back(i2);

            geometry.indices.emplace_back(i0);
            geometry.indices.emplace_back(i2);
            geometry.indices.emplace_back(i3);
        }
    }

    return geometry;
}
