#include "circle.hpp"

#include "../graphics/mesh.hpp"
#include "../render/model.hpp"
#include "utils.hpp"
#include <glm/ext/scalar_constants.hpp>

namespace Physbuzz {

namespace Builtin {

static constexpr Physbuzz::Index MAX_VERTICES = 50;
static constexpr float angleIncrement = (2.0f * glm::pi<float>()) / MAX_VERTICES;

bool ModelCircle::build(const std::shared_ptr<Transfer> transfer) {
    if (ResourceRegistry<Mesh>::contains(Resource)) {
        return true;
    }

    Physbuzz::Mesh::Info<Physbuzz::Model::Vertex> mesh;

    mesh.vertices.reserve(MAX_VERTICES);
    mesh.indices.reserve(MAX_VERTICES * 3);

    std::vector<glm::vec3> positions = std::vector<glm::vec3>(MAX_VERTICES);
    for (Physbuzz::Index i = 0; i < MAX_VERTICES; i++) {
        float angle = i * angleIncrement;

        // source: https://en.wikipedia.org/wiki/Circle#Parametric_form
        positions[i] = {
            glm::cos(angle),
            glm::sin(angle),
            0.0f,
        };
    }

    // triangles generated from the first vertex
    for (Physbuzz::Index i = 1; i < MAX_VERTICES - 1; i++) {
        mesh.indices.insert(mesh.indices.end(), {0, i, i + 1});
    }
    mesh.indices.insert(mesh.indices.end(), {0, MAX_VERTICES - 1, 1});

    std::vector<glm::vec2> texCoords = detail::generate2DTexCoords(positions);
    std::vector<detail::NormalTangent> NT = detail::generate2DNormalTangent(mesh.indices, positions, texCoords);

    for (std::size_t i = 0; i < mesh.vertices.size(); i++) {
        mesh.vertices.emplace_back<Physbuzz::Model::Vertex>({
            .position = positions[i],
            .normal = NT[i].normal,
            .tangent = NT[i].tangent,
            .texCoord0 = texCoords[i],
        });
    }

    return Physbuzz::ResourceRegistry<Physbuzz::Mesh>::insert(Resource, mesh, transfer);
}

} // namespace Builtin

} // namespace Physbuzz
