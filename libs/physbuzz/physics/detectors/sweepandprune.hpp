#pragma once

#include "../../ecs/system.hpp"
#include "../collision.hpp"
#include <list>

namespace Physbuzz {

struct SweepEdge {
    ObjectID object;
    float edge;
    bool isLeft;
};

class SweepAndPrune2D : public CollisionDetector<SweepAndPrune2D> {
  public:
    SweepAndPrune2D(Scene *scene, std::vector<ObjectID> *objects);

  protected:
    using CollisionDetector::findNarrowImpl;

    bool buildImpl();
    bool destroyImpl();

    bool checkImpl(Contact &contact);

    std::vector<Contact> findBroadImpl();

  private:
    std::list<SweepEdge> getEdges();

    std::vector<ObjectID> *m_Objects;

    friend class CollisionDetector<SweepAndPrune2D>;
};

} // namespace Physbuzz
