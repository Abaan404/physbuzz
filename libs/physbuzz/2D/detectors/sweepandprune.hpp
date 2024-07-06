#pragma once

#include "../../collision.hpp"
#include "physbuzz/defines.hpp"
#include <set>

namespace Physbuzz {

struct SweepEdge {
    ObjectID id;
    float edge;
    bool isLeft;
};

class SweepAndPrune2D : public ICollisionDetector {
  public:
    SweepAndPrune2D(Scene &scene);

    void build() override;
    void destroy() override;

    bool check(Contact &contact) override;
    std::list<Contact> find() override;

  protected:
    std::list<SweepEdge> getEdges();

    std::set<ObjectID> m_CollisionObjects;

    EventID m_ObjectAddEventId;
    EventID m_ObjectRemoveEventId;
    std::unordered_map<ObjectID, std::array<EventID, 3>> m_ComponentEventIds;
};

} // namespace Physbuzz
