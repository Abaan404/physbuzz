#include "dynamics.hpp"

#include "../collision/collision.hpp"
#include "../game.hpp"
#include <physbuzz/debug.hpp>
#include <physbuzz/mesh.hpp>
#include <physbuzz/object.hpp>

void Dynamics::tick(Physbuzz::Scene &scene) {
    auto &objects = scene.getObjects();

    for (auto &object : objects) {
        if (!(object.hasComponent<RigidBodyComponent>() && object.hasComponent<TransformableComponent>())) {
            continue;
        }

        ASSERT(object.hasComponent<TransformableComponent>(), "RigidBody object does not have a transform.")
        tickMotion(object);
    }
}

void Dynamics::tickMotion(Physbuzz::Object &object) {
    RigidBodyComponent &body = object.getComponent<RigidBodyComponent>();
    TransformableComponent &transform = object.getComponent<TransformableComponent>();
    float dTime = Game::clock.getDelta() / 1000.0f;

    // apply gravity
    {
        if (!glm::isinf(body.mass)) {
            glm::vec3 force = body.gravity.acceleration * body.mass;
            body.addForce(force);
        }
    }

    // apply drag
    {
        float speed = glm::length(body.velocity);
        if (speed != 0.0f) {
            glm::vec3 force = -glm::normalize(body.velocity) * (body.drag.k1 * speed + body.drag.k2 * speed * speed);
            body.addForce(force);
        }

        body.angular.velocity *= glm::pow(body.angular.drag, dTime);
    }

    // move object wrt S = ut + 1/2 a t**2
    // Note: t**2 is approx 0 for t << 0 (at high framerate)
    {
        translate(object, body.velocity * dTime);
        rotate(object, dTime / 2.0f * glm::quat(0.0f, body.angular.velocity) * transform.orientation);
    }

    // apply accumulated forces to velocity and clear them
    {
        body.angular.velocity += (body.angular.acceleration + body.accumTorques / body.angular.inertia) * dTime;
        body.velocity += (body.acceleration + body.accumForces / body.mass) * dTime;
        body.accumForces = glm::vec3(0.0f, 0.0f, 0.0f);
        body.accumTorques = glm::vec3(0.0f, 0.0f, 0.0f);
    }
}

// not exactly "rotate"s but this works for now
void Dynamics::rotate(Physbuzz::Object &object, const glm::quat delta) {
    TransformableComponent &transform = object.getComponent<TransformableComponent>();
    transform.orientation = glm::normalize(transform.orientation + delta);

    // update mesh
    if (object.hasComponent<Physbuzz::RenderComponent>()) {
        Physbuzz::MeshComponent &mesh = object.getComponent<Physbuzz::MeshComponent>();

        for (int i = 0; i < mesh.screenVertices.size(); i++) {
            glm::quat rotation = transform.orientation * glm::quat(0.0f, mesh.orignalVertices[i]) * glm::inverse(transform.orientation);
            mesh.screenVertices[i] = glm::vec3(rotation.x, rotation.y, rotation.z) + transform.position;
        }

        mesh.scaled = false;

        // adjust collision bounding box
        if (object.hasComponent<BoundingComponent>()) {
            BoundingComponent bounding = {object.getComponent<Physbuzz::MeshComponent>()};
            object.setComponent(bounding);
        }
    }
}

void Dynamics::translate(Physbuzz::Object &object, const glm::vec3 delta) {
    TransformableComponent &transform = object.getComponent<TransformableComponent>();
    transform.translate(delta);

    // update mesh
    if (object.hasComponent<Physbuzz::RenderComponent>()) {
        Physbuzz::MeshComponent &mesh = object.getComponent<Physbuzz::MeshComponent>();

        for (auto &vertex : mesh.screenVertices) {
            vertex += delta;
        }

        mesh.scaled = false;

        // adjust collision bounding box
        if (object.hasComponent<BoundingComponent>()) {
            BoundingComponent bounding = {object.getComponent<Physbuzz::MeshComponent>()};
            object.setComponent(bounding);
        }
    }
}
