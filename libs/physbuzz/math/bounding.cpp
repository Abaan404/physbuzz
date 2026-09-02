#include "bounding.hpp"

namespace Physbuzz {

AABB::AABB() {}

AABB::AABB(const Info &info)
    : m_Info(info) {}

bool AABB::intersects(const glm::vec3 &point) const {
    return point.x >= m_Info.min.x && point.x <= m_Info.max.x &&
           point.y >= m_Info.min.y && point.y <= m_Info.max.y &&
           point.z >= m_Info.min.z && point.z <= m_Info.max.z;
}

bool AABB::intersects(const AABB &aabb) const {
    return m_Info.min.x <= aabb.m_Info.max.x && m_Info.max.x >= aabb.m_Info.min.x &&
           m_Info.min.y <= aabb.m_Info.max.y && m_Info.max.y >= aabb.m_Info.min.y &&
           m_Info.min.z <= aabb.m_Info.max.z && m_Info.max.z >= aabb.m_Info.min.z;
}

const AABB::Info &AABB::getInfo() const {
    return m_Info;
}

AABB AABB::withTransform(const Transform &transform) const {
    // https://www.realtimerendering.com/resources/GraphicsGems/gems/TransBox.c
    AABB::Info result;

    const glm::mat4 model = transform.getModel();

    result.min = glm::vec3(model[3]);
    result.max = glm::vec3(model[3]);

    for (int j = 0; j < 3; j++) {
        glm::vec3 a = glm::vec3{model[j]} * m_Info.min[j];
        glm::vec3 b = glm::vec3{model[j]} * m_Info.max[j];

        result.min += glm::min(a, b);
        result.max += glm::max(a, b);
    }

    return result;
}

void AABB::update(const Info &info) {
    m_Info = info;
}

} // namespace Physbuzz
