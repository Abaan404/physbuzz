#include "builder.hpp"

ObjectBuilder::ObjectBuilder(Physbuzz::Scene *scene)
    : scene(scene) {}

ObjectBuilder::~ObjectBuilder() {}

void ObjectBuilder::generate2DTexCoords(Physbuzz::MeshComponent &mesh) {
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

void ObjectBuilder::generate2DNormals(Physbuzz::MeshComponent &mesh) {
    for (std::size_t i = 0; i < mesh.vertices.size(); ++i) {
        const std::size_t next = (i + 1) % mesh.vertices.size();                            // cycle next vertex
        const glm::vec3 tangent = mesh.vertices[next].position - mesh.vertices[i].position; // get the tangent
        const glm::vec3 normal = glm::cross(tangent, glm::vec3(0.0f, 0.0f, 1.0f));          // cross prod for normal

        mesh.vertices[i].normal = glm::normalize(normal);
    }
}
