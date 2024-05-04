#include "dynamics.hpp"

#include <physbuzz/mesh.hpp>
#include <physbuzz/object.hpp>
#include <vector>

void Dynamics::tick(Physbuzz::Scene &scene) const {
    auto &objects = scene.getObjects();

    for (auto &object : objects) {
        RigidBodyComponent &body = object.getComponent<RigidBodyComponent>();
        TransformableComponent &transform = object.getComponent<TransformableComponent>();
        Physbuzz::MeshComponent &mesh = object.getComponent<Physbuzz::MeshComponent>();

        glm::vec3 delta = body.velocity + 0.5f * body.acceleration;

        std::vector<glm::vec3> vertices = mesh.getVertex();

        for (glm::vec3 &vertex : vertices) {
            vertex += delta;
        }

        mesh.setVertex(vertices);
        transform.position += delta;
        body.velocity += body.acceleration;
    }
}
