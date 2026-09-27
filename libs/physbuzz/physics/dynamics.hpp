#pragma once

#include "../ecs/system.hpp"
#include "../render/defines.hpp"
#include "components/rigidbody.hpp"

namespace Physbuzz {

class Dynamics : public System<TransformComponent, RigidBodyComponent> {
  public:
    struct Info {
        float deltaTime = 0.0f;
        glm::vec3 gravity = {0.0f, 0.0f, 0.0f};
    };

    Dynamics(Scene *scene, const Info &info);
    ~Dynamics();

    void tick();

  private:
    void tickMotion(ObjectID object, float deltaTime) const;

    Info m_Info;
};

} // namespace Physbuzz
