#include "dynamics.hpp"

#include "collision.hpp"

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
    const float &delta = m_Clock.getDelta();

    if (delta > 10.0f) {
        Logger::WARNING("[Dynamics] Max Timeout exceeded, skipping frame. ({:.3f}ms since last tick)", delta);
        return;
    }

    time += delta / 1000.0f;

    while (m_DeltaTime - time <= 0.0f) {
        for (auto &object : m_Objects) {
            tickMotion(scene, object);
        }

        time -= m_DeltaTime;
    }
}

void Dynamics::tickMotion(Scene &scene, ObjectID id) const {
    RigidBodyComponent &body = scene.getComponent<RigidBodyComponent>(id);

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
    // rotate object wrt axis and length of angular velocity vector
    // Note: t**2 is approx 0 for t << 0 (at high framerate)
    {
        MeshComponent &mesh = scene.getComponent<MeshComponent>(id);
        mesh.model.position += body.velocity * m_DeltaTime;
        if (glm::length(body.angular.velocity) > 0.0f) {
            mesh.model.orientation = glm::angleAxis(glm::length(body.angular.velocity) * m_DeltaTime, glm::normalize(body.angular.velocity)) * mesh.model.orientation;
        }

        mesh.model.update();

        // adjust collision bounding box
        AABBComponent aabb = AABBComponent(mesh);
        scene.setComponent(id, aabb);
    }

    // apply accumulated forces to velocity and clear them
    {
        body.angular.velocity += (body.angular.acceleration + body.accumTorques / body.angular.inertia) * m_DeltaTime;
        body.velocity += (body.acceleration + body.accumForces / body.mass) * m_DeltaTime;
        body.accumForces = glm::vec3(0.0f, 0.0f, 0.0f);
        body.accumTorques = glm::vec3(0.0f, 0.0f, 0.0f);
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
