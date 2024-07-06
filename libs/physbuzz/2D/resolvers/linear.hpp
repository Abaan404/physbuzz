#pragma once

#include "../../collision.hpp"

namespace Physbuzz {

class LinearResolver : public ICollisionResolver {
  public:
    LinearResolver(Scene &scene, float restitution);

    void solve(const Contact &contact) override;

  private:
    float m_Restitution;
};

} // namespace Physbuzz
