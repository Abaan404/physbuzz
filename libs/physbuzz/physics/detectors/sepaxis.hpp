#pragma once

#include "../collision.hpp"

namespace Physbuzz {

class SeperatingAxis2D : public CollisionDetector<SeperatingAxis2D> {
  public:
    SeperatingAxis2D(Scene *scene);

  protected:
    using CollisionDetector::findBroadImpl;
    using CollisionDetector::findNarrowImpl;

    bool buildImpl();
    bool destroyImpl();

    bool checkImpl(Contact &contact);

  private:
    float getAxisOverlap(const glm::vec3 &axis, const RigidBodyComponent &body1, const RigidBodyComponent &body2);
    void addMeshNormals(const RigidBodyComponent &body, std::vector<glm::vec3> &axes);

    friend class CollisionDetector<SeperatingAxis2D>;
};

} // namespace Physbuzz
