#include "builder.hpp"

ObjectBuilder::ObjectBuilder(Physbuzz::Scene &scene)
    : scene(scene) {}

ObjectBuilder::~ObjectBuilder() {}

void ObjectBuilder::build() {
    m_Textures.build();

    m_Textures.allocate("circle", "resources/missing.png");
    m_Textures.allocate("quad", "resources/wall.jpg");
}

void ObjectBuilder::destroy() {
    m_Textures.deallocate("quad");
    m_Textures.deallocate("circle");

    m_Textures.destroy();
}

void ObjectBuilder::generate2DTexCoords(const Physbuzz::AABBComponent &aabb, Physbuzz::Mesh &mesh) {
    for (auto &vertex : mesh.vertices) {
        vertex.texCoords = (vertex.position - aabb.min) / (aabb.max - aabb.min);
    }
}

void ObjectBuilder::generate2DNormals(Physbuzz::Mesh &mesh) {
    for (std::size_t i = 0; i < mesh.vertices.size(); ++i) {
        const std::size_t next = (i + 1) % mesh.vertices.size();                            // cycle next vertex
        const glm::vec3 tangent = mesh.vertices[next].position - mesh.vertices[i].position; // get the tangent
        const glm::vec3 normal = glm::cross(tangent, glm::vec3(0.0f, 0.0f, 1.0f));          // cross prod for normal

        mesh.vertices[i].normal = glm::normalize(normal);
    }
}
