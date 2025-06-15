#pragma once

#include "../../collision.hpp"

namespace Physbuzz {

class SeperatingAxis2D : public ICollisionDetector {
  public:
    SeperatingAxis2D(Scene *scene);

    bool check(Contact &contact) override;

  protected:
    float getAxisOverlap(const glm::vec3 &axis, const RenderComponent &render1, const RenderComponent &render2);
    void addMeshNormals(const RenderComponent &render, std::vector<glm::vec3> &axes);
};

} // namespace Physbuzz
