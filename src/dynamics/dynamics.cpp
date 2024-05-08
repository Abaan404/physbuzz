#include "dynamics.hpp"

#include "../collision/collision.hpp"
#include "../game.hpp"
#include <physbuzz/mesh.hpp>
#include <physbuzz/object.hpp>
#include <vector>

void Dynamics::tick(Physbuzz::Scene &scene) {
    auto &objects = scene.getObjects();

    for (auto &object : objects) {
        if (!(object.hasComponent<RigidBodyComponent>() && object.hasComponent<TransformableComponent>())) {
            continue;
        }

        motion(object);
    }
}

void Dynamics::motion(Physbuzz::Object &object) {
    RigidBodyComponent &body = object.getComponent<RigidBodyComponent>();
    float dTime = Game::clock.getDelta() / 1000.0f;

    // apply gravity
    if (object.hasComponent<GravityComponent>()) {
        GravityComponent &gravity = object.getComponent<GravityComponent>();
        glm::vec3 force = gravity.gravity * body.mass;

        addForce(body, force);
    }

    // apply drag
    if (object.hasComponent<DragComponent>()) {
        DragComponent &drag = object.getComponent<DragComponent>();

        float speed = glm::length(body.velocity);
        glm::vec3 force = -glm::normalize(body.velocity) * (drag.k1 * speed + drag.k2 * speed * speed);

        addForce(body, force);
    }

    // S = ut + 1/2 a t**2, but t**2 ~> 0
    displace(object, body.velocity * dTime);

    // apply forces to velocity
    body.velocity += (body.acceleration + body.accumForces / body.mass) * dTime;

    // clear forces
    body.accumForces = glm::vec3(0.0f, 0.0f, 0.0f);
}

void Dynamics::displace(Physbuzz::Object &object, glm::vec3 dDisplacement) {
    // update transform position
    if (object.hasComponent<TransformableComponent>()) {
        TransformableComponent &transform = object.getComponent<TransformableComponent>();
        transform.position += dDisplacement;
    }

    // adjust collision bounding box
    if (object.hasComponent<AABBComponent>()) {
        AABBComponent &bounding = object.getComponent<AABBComponent>();

        bounding.min += dDisplacement;
        bounding.max += dDisplacement;
    }

    // update mesh if any
    if (object.hasComponent<Physbuzz::MeshComponent>()) {
        Physbuzz::MeshComponent &mesh = object.getComponent<Physbuzz::MeshComponent>();

        std::vector<glm::vec3> vertices = mesh.getVertex();
        for (glm::vec3 &vertex : vertices) {
            vertex += dDisplacement;
        }

        mesh.setVertex(vertices);
    }
}

void Dynamics::addForce(RigidBodyComponent &body, const glm::vec3 &force) {
    if (glm::any(glm::isnan(force))) {
        return;
    }

    body.accumForces += force;
}
