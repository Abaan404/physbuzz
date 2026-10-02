#pragma once

#include "../collision.hpp"

namespace Physbuzz {

class Mpr3D : public CollisionDetector<Mpr3D> {
  public:
    Mpr3D(Scene *scene);

  protected:
    bool buildImpl();
    bool destroyImpl();

    bool checkImpl(Contact &contact);

    using CollisionDetector::findBroadImpl;
    using CollisionDetector::findNarrowImpl;

  private:
    friend class CollisionDetector<Mpr3D>;
};

} // namespace Physbuzz
