#include "dynamics.hpp"

#include "collision.hpp"

namespace Physbuzz {

const glm::mat4 TransformableComponent::generateModel() const {
    const glm::mat4 translation = glm::translate(glm::mat4(1.0f), position);
    const glm::mat4 rotation = glm::rotate(glm::mat4(1.0f), glm::angle(orientation), glm::axis(orientation)); // conjugate?
    const glm::mat4 transformation = glm::scale(glm::mat4(1.0f), scale);

    return translation * rotation * transformation;
}

void TransformableComponent::rotate(const glm::quat &delta) {
    orientation = delta * orientation;
}

void TransformableComponent::translate(const glm::vec3 &delta) {
    position += delta;
}

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
        if (glm::length(body.angular.velocity) > 0.0f) {
            rotate(object, glm::angleAxis(glm::length(body.angular.velocity) * m_DeltaTime, glm::normalize(body.angular.velocity)));
        }
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
    transform.rotate(delta);

    // adjust collision bounding box
    if (object.hasComponent<AABBComponent>() && object.hasComponent<Mesh>()) {
        AABBComponent aabb = AABBComponent(object.getComponent<Mesh>(), transform);
        object.setComponent(aabb);
    }
}

void Dynamics::translate(Object &object, const glm::vec3 &delta) const {
    TransformableComponent &transform = object.getComponent<TransformableComponent>();
    transform.translate(delta);

    // adjust collision bounding box
    if (object.hasComponent<AABBComponent>()) {
        AABBComponent &aabb = object.getComponent<AABBComponent>();
        aabb.max += delta;
        aabb.min += delta;
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
