#include "plane.hpp"

namespace Physbuzz {

Plane::Plane(const Info &info)
    : m_Info(info) {}

float Plane::signedDistance(const glm::vec3 &point) const {
    return glm::dot(m_Info.normal, point) + m_Info.distance;
}

const Plane::Info &Plane::getInfo() const {
    return m_Info;
}

} // namespace Physbuzz
