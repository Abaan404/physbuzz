#pragma once

#include "../components/rigidbody.hpp"

namespace Physbuzz {

inline glm::vec3 minkowskiSupportPoint(const RigidBodyComponent &body1, const Transform &transform1, const RigidBodyComponent &body2, const Transform &transform2, const glm::vec3 &direction) {
    return body2.calcSupportPoint(direction, transform2) - body1.calcSupportPoint(-direction, transform1);
}

} // namespace Physbuzz
