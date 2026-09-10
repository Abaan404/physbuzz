#pragma once

#include "../ecs/system.hpp"
#include "../render/defines.hpp"
#include "components/rigidbody.hpp"

namespace Physbuzz {

class Dynamics : public System<TransformComponent, RigidBodyComponent> {
  public:
    Dynamics(Scene *scene, float dtime);
    ~Dynamics();

    void tick();

    const bool &isRunning() const;
    void start();
    void stop();
    const bool &toggle();

  private:
    void tickMotion(ObjectID id) const;

    glm::vec3 m_Gravity = {0.0f, 0.0f, 0.0f};

    float m_DeltaTime = 0.0f;
    bool m_IsRunning = false;
};

} // namespace Physbuzz
