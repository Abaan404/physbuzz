#pragma once

#include "../collision.hpp"

namespace Physbuzz {

class SimpleResolver3D : public CollisionResolver<SimpleResolver3D> {
  public:
    struct Info {
        float restitution = 1.0f;
    };

    SimpleResolver3D(Scene *scene, const Info &info);

  private:
    bool buildImpl();
    bool destroyImpl();

    void solveImpl(const Contact &contact);

    Info m_Info;

    friend class CollisionResolver<SimpleResolver3D>;
};

} // namespace Physbuzz
