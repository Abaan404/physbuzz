#include "dynamics.hpp"

#include "../ecs/scene.hpp"
#include "../misc/clock.hpp"
#include "collision.hpp"
#include <tracy/Tracy.hpp>

namespace Physbuzz {

Dynamics::Dynamics(Scene *scene, float dtime)
    : System<RenderComponent, RigidBodyComponent>(scene),
      m_DeltaTime(dtime) {}

Dynamics::~Dynamics() {}

void Dynamics::tick() {
    ZoneScopedN("Dynamics/Tick");

    if (!m_IsRunning) {
        return;
    }

    static float time = 0.0f;
    const float &delta = m_Scene->getSystem<Clock>()->getDelta();

    if (delta > 10e3f) {
        Logger::WARNING("[Dynamics] Max Timeout exceeded, skipping frame. ({:.3f}ms since last tick)", delta / 1e3f);
        return;
    }

    time += delta / 1e6f;

    while (m_DeltaTime - time <= 0.0f) {
        for (auto &object : m_Objects) {
            tickMotion(object);
        }

        time -= m_DeltaTime;
    }
}

void Dynamics::tickMotion(ObjectID id) const {
    const auto [body, render] = m_Scene->getComponent<RigidBodyComponent, RenderComponent>(id);

    RigidBodyComponent::Info info = body.getInfo();

    // apply gravity
    {
        if (!glm::isinf(info.mass)) {
            glm::vec3 force = m_Gravity * info.mass;
            body.addForce(force);
        }
    }

    // apply drag
    {
        float speed = glm::length(info.velocity);
        if (speed != 0.0f) {
            glm::vec3 force = -glm::normalize(info.velocity) * (info.drag.k1 * speed + info.drag.k2 * speed * speed);
            body.addForce(force);
        }

        info.angular.velocity *= glm::pow(info.angular.drag, m_DeltaTime);
    }

    // move object wrt S = ut + 1/2 a t**2
    // rotate object wrt axis and length of angular velocity vector
    // Note: t**2 is approx 0 for t << 0 (at high framerate)
    {
        Transform::Info transform = render.transform.getInfo();
        transform.position += info.velocity * m_DeltaTime;
        if (glm::length(info.angular.velocity) > 0.0f) {
            transform.orientation = glm::angleAxis(glm::length(info.angular.velocity) * m_DeltaTime, glm::normalize(info.angular.velocity)) * transform.orientation;
        }

        render.transform.update(transform);
    }

    // adjust bounding box
    {
        if (m_Scene->containsComponent<BoundingComponent>(id)) {
            auto [bounding] = m_Scene->getComponent<BoundingComponent>(id);
            bounding.aabb = body.getAABB().withTransform(render.transform);
        }
    }

    // apply accumulated info to velocity and clear them
    {
        info.angular.velocity += (info.angular.acceleration + info.torques / info.angular.inertia) * m_DeltaTime;
        info.velocity += (info.acceleration + info.forces / info.mass) * m_DeltaTime;
        info.forces = glm::vec3(0.0f, 0.0f, 0.0f);
        info.torques = glm::vec3(0.0f, 0.0f, 0.0f);
    }

    body.update(info);
}

void Dynamics::start() {
    m_IsRunning = true;
}

void Dynamics::stop() {
    m_IsRunning = false;
}

const bool &Dynamics::toggle() {
    return m_IsRunning ^= true;
}

const bool &Dynamics::isRunning() const {
    return m_IsRunning;
}

} // namespace Physbuzz
