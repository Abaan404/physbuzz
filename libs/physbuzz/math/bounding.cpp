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

} // namespace Physbuzz
