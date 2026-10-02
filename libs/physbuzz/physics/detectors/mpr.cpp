#include "mpr.hpp"

#include "../../math/utils.hpp"
#include "../../render/defines.hpp"
#include "../components/rigidbody.hpp"
#include "utils.hpp"

// implemented from https://archive.org/details/isbn_9781584505273 pg165

namespace Physbuzz {

Mpr3D::Mpr3D(Scene *scene) : CollisionDetector(scene) {}

bool Mpr3D::buildImpl() {
    return true;
}

bool Mpr3D::destroyImpl() {
    return true;
}

bool Mpr3D::checkImpl(Contact &contact) {
    const auto [body1, transform1] = m_Scene->getComponent<RigidBodyComponent, TransformComponent>(contact.object1);
    const auto [body2, transform2] = m_Scene->getComponent<RigidBodyComponent, TransformComponent>(contact.object2);

    // find_origin_ray();
    glm::vec3 V0 = transform2.transform.getInfo().position - transform1.transform.getInfo().position;

    // find_candidate_portal();
    glm::vec3 direction = -V0;
    glm::vec3 V1 = minkowskiSupportPoint(body1, transform1.transform, body2, transform2.transform, direction);

    direction = glm::cross(V1, V0);
    if (glm::length(direction) < 1e-4f) {
        return false;
    }

    glm::vec3 V2 = minkowskiSupportPoint(body1, transform1.transform, body2, transform2.transform, direction);

    direction = glm::cross(V1 - V0, V2 - V0);
    if (glm::length(direction) < 1e-4f) {
        return false;
    }

    glm::vec3 V3 = minkowskiSupportPoint(body1, transform1.transform, body2, transform2.transform, direction);

    // while (origin ray does not intersect candidate)
    // choose_new_candidate();
    while (true) {
        direction = glm::cross(V3 - V0, V2 - V0);
        if (isFacing(-direction, V0)) {
            V1 = minkowskiSupportPoint(body1, transform1.transform, body2, transform2.transform, direction);
            std::swap(V2, V3);
            continue;
        }

        direction = glm::cross(V1 - V0, V3 - V0);
        if (isFacing(-direction, V0)) {
            V2 = minkowskiSupportPoint(body1, transform1.transform, body2, transform2.transform, direction);
            std::swap(V1, V3);
            continue;
        }

        direction = glm::cross(V2 - V0, V1 - V0);
        if (isFacing(-direction, V0)) {
            V3 = minkowskiSupportPoint(body1, transform1.transform, body2, transform2.transform, direction);
            std::swap(V1, V2);
            continue;
        }

        break;
    }

    while (true) {
        const glm::vec3 n123 = glm::normalize(glm::cross(V1 - V3, V2 - V3));

        // if (origin inside portal)
        if (glm::dot(n123, V1) >= 0.0f) {
            // return hit;
            return true;
        }

        // find_support_in_direction_of_portal();
        glm::vec3 V4 = minkowskiSupportPoint(body1, transform1.transform, body2, transform2.transform, n123);

        // if (origin outside support plane)
        if (!isFacing(V4, n123)) {
            // return miss;
            return false;
        }

        // if (Support plane close to portal)
        if (!isFacing<1e-4f>(V4, n123)) {
            // return miss;
            return false;
        }

        // choose_new_portal();
        const bool inside1 = isFacing(glm::cross(V1 - V0, V4 - V0), V0);
        const bool inside2 = isFacing(glm::cross(V2 - V0, V4 - V0), V0);
        const bool inside3 = isFacing(glm::cross(V3 - V0, V4 - V0), V0);

        if (inside1) {
            if (inside2) {
                V1 = V4;
            } else {
                V3 = V4;
            }
        } else {
            if (inside3) {
                V2 = V4;
            } else {
                V1 = V4;
            }
        }
    }

    return false;
}

} // namespace Physbuzz
