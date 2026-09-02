#pragma once

#include "transform.hpp"
#include <glm/glm.hpp>

namespace Physbuzz {

class AABB {
  public:
    struct Info {
        glm::vec3 min = glm::vec3(std::numeric_limits<float>::max());
        glm::vec3 max = glm::vec3(std::numeric_limits<float>::lowest());
    };

    AABB();
    AABB(const Info &info);

    bool intersects(const glm::vec3 &point) const;
    bool intersects(const AABB &aabb) const;

    AABB withTransform(const Transform &transform) const;

    void update(const Info &info);

    const Info &getInfo() const;

  private:
    Info m_Info;
};

} // namespace Physbuzz
