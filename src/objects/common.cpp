#include "common.hpp"

std::vector<glm::vec2> generateTexCoords(const std::vector<glm::vec3> &positions) {
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

std::vector<glm::vec3> generateNormals(const std::vector<Physbuzz::Index> &indices, const std::vector<glm::vec3> &positions) {
    std::vector<glm::vec3> normals = std::vector<glm::vec3>(positions.size());

    for (std::size_t i = 0; i < indices.size(); i += 3) {
        const Physbuzz::Index i0 = indices[i];
        const Physbuzz::Index i1 = indices[i + 1];
        const Physbuzz::Index i2 = indices[i + 2];

        const glm::vec3 &p1 = positions[i0];
        const glm::vec3 &p2 = positions[i1];
        const glm::vec3 &p3 = positions[i2];

        const glm::vec3 p12 = p2 - p1;
        const glm::vec3 p13 = p3 - p1;
        const glm::vec3 normal = glm::cross(p12, p13);

        normals[i0] += normal;
        normals[i1] += normal;
        normals[i2] += normal;
    }

    for (auto &normal : normals) {
        normal = glm::normalize(normal);
    }

    return normals;
}
