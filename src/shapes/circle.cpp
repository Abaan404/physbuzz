#include "circle.hpp"

#include "utils.hpp"
#include <glm/ext/scalar_constants.hpp>

Physbuzz::Geometry<Physbuzz::Model::Vertex> ModelCircle::build() {
    Physbuzz::Geometry<Physbuzz::Model::Vertex> geometry;

    constexpr Physbuzz::Index maxVertices = 50;

    std::uint32_t vertexCount = maxVertices;
    std::uint32_t indexCount = (maxVertices - 2) * 3;

    geometry.vertices.reserve(vertexCount);
    geometry.indices.reserve(indexCount);

    std::vector<glm::vec3> positions = std::vector<glm::vec3>(maxVertices);
    for (Physbuzz::Index i = 0; i < maxVertices; i++) {
        constexpr float angleIncrement = (2.0f * glm::pi<float>()) / maxVertices;

        float angle = i * angleIncrement;

        // source: https://en.wikipedia.org/wiki/Circle#Parametric_form
        positions[i] = {
            glm::cos(angle),
            glm::sin(angle),
            0.0f,
        };
    }

    // triangles generated from the first vertex
    for (Physbuzz::Index i = 1; i < maxVertices - 1; i++) {
        geometry.indices.insert(geometry.indices.end(), {0, i, i + 1});
    }

    std::vector<glm::vec2> texCoords = detail::generate2DTexCoords(positions);
    std::vector<detail::NormalTangent> NT = detail::generate2DNormalTangent(geometry.indices, positions, texCoords);

    for (std::size_t i = 0; i < maxVertices; i++) {
        geometry.vertices.emplace_back<Physbuzz::Model::Vertex>({
            .position = positions[i],
            .normal = NT[i].normal,
            .tangent = NT[i].tangent,
            .texCoord0 = texCoords[i],
        });
    }

    return geometry;
}
