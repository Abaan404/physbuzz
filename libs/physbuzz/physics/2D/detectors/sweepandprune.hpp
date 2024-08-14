#pragma once

#include "../../../ecs/system.hpp"
#include "../../collision.hpp"

namespace Physbuzz {

struct SweepEdge {
    ObjectID object;
    float edge;
    bool isLeft;
};

class SweepAndPrune2D : public ICollisionDetector {
  public:
    SweepAndPrune2D(Scene *scene, std::set<ObjectID> *objects);

    bool check(Contact &contact) override;
    std::list<Contact> find() override;

  protected:
    std::list<SweepEdge> getEdges();

    std::set<ObjectID> *m_Objects;
};

} // namespace Physbuzz
