#pragma once

#include "../../collision.hpp"
#include "../../dynamics.hpp"

namespace Physbuzz {

class LinearResolver2D : public ICollisionResolver {
  public:
    LinearResolver2D(Scene *scene, float restitution);

    void solve(const Contact &contact) override;

  protected:
    const glm::vec3 calcImpulse(const RigidBodyComponent &body1, const RigidBodyComponent &body2, const Contact &contact);

    float m_Restitution;
};

} // namespace Physbuzz
