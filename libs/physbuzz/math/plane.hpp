#pragma once

#include <glm/glm.hpp>

namespace Physbuzz {

class Plane {
  public:
    struct Info {
        glm::vec3 normal = {0.0f, 0.0f, 1.0f};
        float distance = {0.0f};
    };

    Plane();
    Plane(const Info &info);

    float signedDistance(const glm::vec3 &point) const;

    const Info &getInfo() const;

  private:
    Info m_Info;
};

} // namespace Physbuzz
