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

void ObjectBuilder::applyTransformsToMesh(const Physbuzz::TransformableComponent &transform, Physbuzz::Mesh &mesh) {
    for (auto &vertex : mesh.positions) {
        vertex = transform.orientation * vertex + transform.position;
    }

    for (auto &vertex : mesh.vertices) {
        vertex.normal = transform.orientation * vertex.normal;
    }
}

void ObjectBuilder::generate2DTexCoords(const Physbuzz::BoundingComponent &bounding, Physbuzz::Mesh &mesh) {
    for (std::size_t i = 0; i < mesh.positions.size(); ++i) {
        mesh.vertices[i].texCoords = (mesh.positions[i] - bounding.aabb.min) / (bounding.aabb.max - bounding.aabb.min);
    }
}

void ObjectBuilder::generate2DNormals(Physbuzz::Mesh &mesh) {
    for (std::size_t i = 0; i < mesh.positions.size(); ++i) {
        const std::size_t next = (i + 1) % mesh.positions.size();                  // cycle next vertex
        const glm::vec3 tangent = mesh.positions[next] - mesh.positions[i];        // get the tangent
        const glm::vec3 normal = glm::cross(tangent, glm::vec3(0.0f, 0.0f, 1.0f)); // cross prod for normal

        mesh.vertices[i].normal = glm::normalize(normal);
    }
}
