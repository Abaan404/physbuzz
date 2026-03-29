#pragma once

#include <glm/glm.hpp>

namespace Physbuzz {

class Plane {
  public:
    struct Info {
        glm::vec3 normal;
        float distance;
    };

    Plane(const Info &info);

    float signedDistance(const glm::vec3 &point) const;

    const Info &getInfo() const;

  private:
    Info m_Info;
};

} // namespace Physbuzz
