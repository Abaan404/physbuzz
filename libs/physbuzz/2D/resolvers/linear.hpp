#pragma once

#include "../../collision.hpp"

namespace Physbuzz {

class LinearResolver : public ICollisionResolver {
  public:
    LinearResolver(float restitution);

    void solve(Scene &scene, const Contact &contact) override;

  private:
    float m_Restitution;
};

} // namespace Physbuzz
