#include "dynamics.hpp"

#include "../collision/collision.hpp"
#include <physbuzz/mesh.hpp>
#include <physbuzz/object.hpp>
#include <vector>

void Dynamics::tick(Physbuzz::Scene &scene) {
    auto &objects = scene.getObjects();

    for (auto &object : objects) {
        motion(object);
        gravity(object);
    }
}

void Dynamics::motion(Physbuzz::Object &object) {
    if (!(object.hasComponent<RigidBodyComponent>() && object.hasComponent<TransformableComponent>())) {
        return;
    }

    RigidBodyComponent &body = object.getComponent<RigidBodyComponent>();
    TransformableComponent &transform = object.getComponent<TransformableComponent>();
    AABBComponent &bounding = object.getComponent<AABBComponent>();

    glm::vec3 delta = body.velocity + 0.5f * body.acceleration;
    transform.position += delta;
    bounding.min += delta;
    bounding.max += delta;
    body.velocity += body.acceleration;

    if (!object.hasComponent<Physbuzz::MeshComponent>()) {
        return;
    }

    Physbuzz::MeshComponent &mesh = object.getComponent<Physbuzz::MeshComponent>();

    std::vector<glm::vec3> vertices = mesh.getVertex();
    for (glm::vec3 &vertex : vertices) {
        vertex += delta;
    }

    mesh.setVertex(vertices);
}

void Dynamics::gravity(Physbuzz::Object &object) {
    if (!(object.hasComponent<RigidBodyComponent>() && object.hasComponent<TransformableComponent>())) {
        return;
    }

    RigidBodyComponent &body = object.getComponent<RigidBodyComponent>();
    TransformableComponent &transform = object.getComponent<TransformableComponent>();

    body.acceleration += m_Gravity;
}
