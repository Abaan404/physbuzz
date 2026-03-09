#include "circle.hpp"

#include "../graphics/mesh.hpp"
#include "../graphics/model.hpp"
#include "utils.hpp"
#include <glm/ext/scalar_constants.hpp>

namespace Physbuzz {

namespace Builtin {

bool ModelCircle::build(TransferBatch &batch) {
    if (ResourceRegistry<Mesh>::contains(Resource)) {
        return true;
    }

    constexpr Index maxVertices = 50;

    Mesh::Info info = {
        .description = &Model::Vertex::Description,
        .vertexCount = maxVertices,
        .indexCount = (maxVertices - 2) * 3,
    };

    if (!ResourceRegistry<Mesh>::insert(Resource, info)) {
        return false;
    }

    std::vector<Model::Vertex> vertices;
    std::vector<Index> indices;

    vertices.reserve(info.vertexCount);
    indices.reserve(info.indexCount);

    std::vector<glm::vec3> positions = std::vector<glm::vec3>(maxVertices);
    for (Index i = 0; i < maxVertices; i++) {
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
    for (Index i = 1; i < maxVertices - 1; i++) {
        indices.insert(indices.end(), {0, i, i + 1});
    }

    std::vector<glm::vec2> texCoords = detail::generate2DTexCoords(positions);
    std::vector<detail::NormalTangent> NT = detail::generate2DNormalTangent(indices, positions, texCoords);

    for (std::size_t i = 0; i < maxVertices; i++) {
        vertices.emplace_back<Model::Vertex>({
            .position = positions[i],
            .normal = NT[i].normal,
            .tangent = NT[i].tangent,
            .texCoord0 = texCoords[i],
        });
    }

    return Resource->write(std::move(vertices), std::move(indices), batch);
}

} // namespace Builtin

} // namespace Physbuzz
