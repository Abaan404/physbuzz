#include "dynamics.hpp"

#include "../ecs/scene.hpp"
#include "../misc/clock.hpp"
#include "collision.hpp"
#include "components/gravity.hpp"
#include <tracy/Tracy.hpp>

namespace Physbuzz {

Dynamics::Dynamics(Scene *scene, const Info &info)
    : System<TransformComponent, RigidBodyComponent>(scene),
      m_Info(info) {}

Dynamics::~Dynamics() {}

void Dynamics::tick() {
    ZoneScopedN("Dynamics/Tick");

    static float time = 0.0f;
    const float &deltaMicros = m_Scene->getSystem<Clock>()->getDelta();

    if (deltaMicros > 10e3f) {
        Logger::WARNING("[Dynamics] Max Timeout exceeded, skipping physics frame. ({:.3f}ms since last tick)", deltaMicros / 1e3f);
        return;
    }

    time += deltaMicros / 1e6f;

    while (m_Info.deltaTime - time <= 0.0f) {
        for (auto &object : m_Objects) {
            tickMotion(object, m_Info.deltaTime);
        }

        time -= m_Info.deltaTime;
    }
}

void Dynamics::tickMotion(ObjectID object, float deltaTime) const {
    const auto [body, transform] = m_Scene->getComponent<RigidBodyComponent, TransformComponent>(object);

    RigidBodyComponent::Info info = body.getInfo();

    // apply gravity
    if (m_Scene->containsComponent<GravityComponent>(object)) {
        if (info.inverseMass > 0.0f) {
            info.forces += m_Info.gravity / info.inverseMass;
        }
    }

    // apply drag
    {
        float speed = glm::length(info.velocity);

        if (speed > 0.0f) {
            info.forces += -glm::normalize(info.velocity) * (info.drag.k1 * speed + info.drag.k2 * speed * speed);
        }

        info.angular.velocity *= glm::pow(info.angular.drag, deltaTime);
    }

    // apply accumulated torques and linear forces to velocity and clear them
    {
        info.angular.velocity += (info.angular.acceleration + info.angular.inverseInertia * info.torques) * deltaTime;
        info.velocity += (info.acceleration + info.forces * info.inverseMass) * deltaTime;

        info.forces = glm::vec3(0.0f);
        info.torques = glm::vec3(0.0f);
    }

    // move object wrt S = ut + 1/2 a t**2
    // rotate object wrt axis and length of angular velocity vector
    // Note: t**2 is approx 0 for t << 0 (at high framerate)
    {
        Transform::Info transformInfo = transform.transform.getInfo();

        transformInfo.position += info.velocity * deltaTime;

        if (glm::length(info.angular.velocity) > 0.0f) {
            transformInfo.orientation = glm::angleAxis(glm::length(info.angular.velocity) * deltaTime, glm::normalize(info.angular.velocity)) * transformInfo.orientation;
        }

        body.applyTransform(transformInfo);
        transform.transform.update(transformInfo);
    }

    body.update(info);
}

} // namespace Physbuzz
