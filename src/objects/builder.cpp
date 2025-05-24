#include "builder.hpp"

ObjectBuilder::ObjectBuilder(Physbuzz::Scene *scene)
    : scene(scene) {}

ObjectBuilder::~ObjectBuilder() {}

void ObjectBuilder::generateTexCoords(Physbuzz::Mesh &mesh) {
    glm::vec3 min = glm::vec3(std::numeric_limits<float>::max());
    glm::vec3 max = glm::vec3(std::numeric_limits<float>::lowest());

    // impose the entire texture onto the mesh
    for (const auto &vertex : mesh.vertices) {
        min = glm::min(min, vertex.position);
        max = glm::max(max, vertex.position);
    }

    for (auto &vertex : mesh.vertices) {
        vertex.texCoords = (vertex.position - min) / (max - min);
    }
}

void ObjectBuilder::generateNormals(Physbuzz::Mesh &mesh) {
    for (std::size_t i = 0; i < mesh.indices.size(); i += 3) {
        const Physbuzz::Index i0 = mesh.indices[i];
        const Physbuzz::Index i1 = mesh.indices[i + 1];
        const Physbuzz::Index i2 = mesh.indices[i + 2];

        const glm::vec3 &p1 = mesh.vertices[i0].position;
        const glm::vec3 &p2 = mesh.vertices[i1].position;
        const glm::vec3 &p3 = mesh.vertices[i2].position;

        const glm::vec3 p12 = p2 - p1;
        const glm::vec3 p13 = p3 - p1;
        const glm::vec3 normal = glm::cross(p12, p13);

        mesh.vertices[i0].normal += normal;
        mesh.vertices[i1].normal += normal;
        mesh.vertices[i2].normal += normal;
    }

    for (auto &v : mesh.vertices) {
        v.normal = glm::normalize(v.normal);
    }
}
