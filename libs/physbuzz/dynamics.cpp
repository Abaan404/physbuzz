#include "dynamics.hpp"

#include "collision.hpp"
#include "mesh.hpp"

namespace Physbuzz {

Dynamics::Dynamics(Clock &clock)
    : m_Clock(clock) {}

Dynamics::~Dynamics() {}

const Clock &Dynamics::getClock() const {
    return m_Clock;
}

void Dynamics::tick(Scene &scene) {
    auto &objects = scene.getObjects();
    m_Clock.tick();

    for (auto &object : objects) {
        if (!(object.hasComponent<RigidBodyComponent>() && object.hasComponent<TransformableComponent>())) {
            continue;
        }

        Logger::ASSERT(object.hasComponent<TransformableComponent>(), "RigidBody object does not have a transform.");
        tickMotion(object);
    }
}

void Dynamics::tickMotion(Object &object) {
    RigidBodyComponent &body = object.getComponent<RigidBodyComponent>();
    TransformableComponent &transform = object.getComponent<TransformableComponent>();
    float dTime = m_Clock.getDelta() / 1000.0f;

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
void Dynamics::rotate(Object &object, const glm::quat delta) {
    TransformableComponent &transform = object.getComponent<TransformableComponent>();
    transform.orientation = glm::normalize(transform.orientation + delta);

    // update mesh
    if (object.hasComponent<RenderComponent>()) {
        MeshComponent &mesh = object.getComponent<MeshComponent>();
        const std::vector<glm::vec3> &originalVertices = mesh.getOriginalVertices();

        for (int i = 0; i < mesh.vertices.size(); i++) {
            glm::quat rotation = transform.orientation * glm::quat(0.0f, originalVertices[i]) * glm::inverse(transform.orientation);
            mesh.vertices[i] = glm::vec3(rotation.x, rotation.y, rotation.z) + transform.position;
        }

        mesh.renormalize();

        // adjust collision bounding box
        if (object.hasComponent<BoundingComponent>()) {
            BoundingComponent bounding = object.getComponent<MeshComponent>();
            bounding.build(mesh);

            object.setComponent(bounding);
        }
    }
}

void Dynamics::translate(Object &object, const glm::vec3 delta) {
    TransformableComponent &transform = object.getComponent<TransformableComponent>();
    transform.translate(delta);

    // update mesh
    if (object.hasComponent<RenderComponent>()) {
        MeshComponent &mesh = object.getComponent<MeshComponent>();

        for (auto &vertex : mesh.vertices) {
            vertex += delta;
        }

        mesh.renormalize();

        // adjust collision bounding box
        if (object.hasComponent<BoundingComponent>()) {
            BoundingComponent bounding = object.getComponent<MeshComponent>();
            AABBComponent box = bounding.getBox();

            box.max += delta;
            box.min += delta;
            bounding.build(box);

            object.setComponent(bounding);
        }
    }
}

} // namespace Physbuzz
