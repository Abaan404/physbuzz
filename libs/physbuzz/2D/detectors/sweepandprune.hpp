#pragma once

#include "../../collision.hpp"

namespace Physbuzz {

class SweepAndPrune2D : public ICollisionDetector {
  public:
    bool check(Scene &scene, Contact &contact) override;

    std::list<Contact> find(Scene &scene) override;
    void find(Scene &scene, std::list<Contact> &contacts) override;
    void reset() override;

  protected:
    void sortObjects(Scene &scene);

    std::vector<std::pair<ObjectID, AABBComponent>> m_SortedObjects;
};

} // namespace Physbuzz
