#include "dynamics.hpp"

#include "collision.hpp"
#include "mesh.hpp"

namespace Physbuzz {

Dynamics::Dynamics(float dtime)
    : m_DeltaTime(dtime) {}

Dynamics::~Dynamics() {}

const Clock &Dynamics::getClock() const {
    return m_Clock;
}

void Dynamics::tick(Scene &scene) {
    if (!m_IsRunning) {
        return;
    }

    m_Clock.tick();
    static float time = 0.0f;
    const float &delta = m_Clock.getDelta().count();

    if (delta > 10.0f) {
        Logger::WARNING(std::format("[Dynamics] Max Timeout exceeded, skipping frame ({}ms since last tick)", delta));
        return;
    }

    time += delta / 1000.0f;
    auto &objects = scene.getObjects();
    while (m_DeltaTime - time <= 0.0f) {
        for (auto &object : objects) {
            if (!(object.hasComponent<RigidBodyComponent>() && object.hasComponent<TransformableComponent>())) {
                continue;
            }

            Logger::ASSERT(object.hasComponent<TransformableComponent>(), "RigidBody object does not have a transform.");
            tickMotion(object);
        }

        time -= m_DeltaTime;
    }
}

void Dynamics::tickMotion(Object &object) const {
    RigidBodyComponent &body = object.getComponent<RigidBodyComponent>();
    TransformableComponent &transform = object.getComponent<TransformableComponent>();

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

        body.angular.velocity *= glm::pow(body.angular.drag, m_DeltaTime);
    }

    // move object wrt S = ut + 1/2 a t**2
    // Note: t**2 is approx 0 for t << 0 (at high framerate)
    {
        translate(object, body.velocity * m_DeltaTime);
        rotate(object, m_DeltaTime / 2.0f * glm::quat(0.0f, body.angular.velocity) * transform.orientation);
    }

    // apply accumulated forces to velocity and clear them
    {
        body.angular.velocity += (body.angular.acceleration + body.accumTorques / body.angular.inertia) * m_DeltaTime;
        body.velocity += (body.acceleration + body.accumForces / body.mass) * m_DeltaTime;
        body.accumForces = glm::vec3(0.0f, 0.0f, 0.0f);
        body.accumTorques = glm::vec3(0.0f, 0.0f, 0.0f);
    }
}

// not exactly "rotate"s but this works for now
void Dynamics::rotate(Object &object, const glm::quat &delta) const {
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

void Dynamics::translate(Object &object, const glm::vec3 &delta) const {
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

void Dynamics::start() {
    m_Clock.tick();
    m_IsRunning = true;
}

void Dynamics::stop() {
    m_IsRunning = false;
}

const bool &Dynamics::toggle() {
    m_Clock.tick();
    return m_IsRunning ^= true;
}

const bool &Dynamics::isRunning() const {
    return m_IsRunning;
}

} // namespace Physbuzz
