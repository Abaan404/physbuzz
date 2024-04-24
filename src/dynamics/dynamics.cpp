#include "dynamics.hpp"

#include "../renderer/mesh.hpp"
#include "../scene/object.hpp"
#include <vector>

Dynamics::Dynamics(Scene &scene) : scene(scene) {}

void Dynamics::tick() {
    auto &objects = scene.get_objects();

    for (auto &object : objects) {
        RigidBodyComponent &body = object.get_component<RigidBodyComponent>();
        TransformableComponent &transform = object.get_component<TransformableComponent>();
        MeshComponent &mesh = object.get_component<MeshComponent>();

        glm::vec3 delta = body.velocity + 0.5f * body.acceleration;

        std::vector<glm::vec3> vertices = mesh.get_vertex();

        for (glm::vec3 &vertex : vertices) {
            vertex += delta;
        }

        mesh.set_vertex(vertices);
        transform.position += delta;
    }
}
