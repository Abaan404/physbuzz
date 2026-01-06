#include "utils.hpp"

namespace Physbuzz {

namespace detail {

std::vector<glm::vec2> generate2DTexCoords(const std::vector<glm::vec3> &positions) {
    glm::vec3 min = glm::vec3(std::numeric_limits<float>::max());
    glm::vec3 max = glm::vec3(std::numeric_limits<float>::lowest());

    // impose the entire texture onto the mesh
    for (const auto &position : positions) {
        min = glm::min(min, position);
        max = glm::max(max, position);
    }

    std::vector<glm::vec2> texCoords;

    for (auto &position : positions) {
        texCoords.emplace_back((position - min) / (max - min));
    }

    return texCoords;
}

std::vector<NormalTangent> generate2DNormalTangent(const std::vector<Physbuzz::Index> &indices, const std::vector<glm::vec3> &positions, const std::vector<glm::vec2> &texCoords) {
    std::vector<NormalTangent> NT = std::vector<NormalTangent>(positions.size());

    if (indices.size() % 3 != 0) {
        return NT;
    }

    for (std::size_t i = 0; i < indices.size(); i += 3) {
        const Physbuzz::Index i0 = indices[i];
        const Physbuzz::Index i1 = indices[i + 1];
        const Physbuzz::Index i2 = indices[i + 2];

        const glm::vec3 &p1 = positions[i0];
        const glm::vec3 &p2 = positions[i1];
        const glm::vec3 &p3 = positions[i2];

        const glm::vec2 uv1 = texCoords[i0];
        const glm::vec2 uv2 = texCoords[i1];
        const glm::vec2 uv3 = texCoords[i2];

        const glm::vec3 p12 = p2 - p1;
        const glm::vec3 p13 = p3 - p1;

        const glm::vec2 dUV1 = uv2 - uv1;
        const glm::vec2 dUV2 = uv3 - uv1;

        float det = dUV1.x * dUV2.y - dUV2.x * dUV1.y;
        if (glm::abs(det) < 1e-8f) {
            det = 1.0f;
        }

        const glm::vec3 normal = glm::cross(p12, p13);
        const glm::vec3 tangent = 1.0f / det * (dUV2.y * p12 - dUV1.y * p13);

        NT[i0].normal += normal;
        NT[i1].normal += normal;
        NT[i2].normal += normal;
        NT[i0].tangent += tangent;
        NT[i1].tangent += tangent;
        NT[i2].tangent += tangent;
    }

    for (auto &nt : NT) {
        if (glm::dot(nt.normal, nt.normal) > 0.0f) {
            nt.normal = glm::normalize(nt.normal);
        } else {
            nt.normal = glm::vec3(0, 0, 1);
        }

        glm::vec3 tangent = nt.tangent;
        tangent -= nt.normal * glm::dot(nt.normal, tangent);

        if (glm::dot(tangent, tangent) > 0.0f) {
            nt.tangent = glm::normalize(tangent);
        } else {
            nt.tangent = glm::vec3(1, 0, 0);
        }
    }

    return NT;
}

} // namespace detail

} // namespace Physbuzz
