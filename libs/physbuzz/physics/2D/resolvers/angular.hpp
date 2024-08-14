#pragma once

#include "linear.hpp"

namespace Physbuzz {

class AngularResolver2D : public LinearResolver2D {
  public:
    AngularResolver2D(Scene *scene, float restitution);

    void solve(const Contact &contact) override;

  protected:
    const glm::vec3 calcTorque(const RigidBodyComponent &body, const TransformableComponent &transform, const glm::vec3 &point, const glm::vec3 &impulse);
};

} // namespace Physbuzz
