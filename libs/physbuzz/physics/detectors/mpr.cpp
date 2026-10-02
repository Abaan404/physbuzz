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
        contact.normal = glm::normalize(V1);
        contact.depth = glm::length(V1);
        contact.point = body1.calcSupportPoint(contact.normal, transform1.transform);

        return true;
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

    bool hit = false;

    while (true) {
        direction = glm::normalize(glm::cross(V1 - V3, V2 - V3));

        // if (origin inside portal)
        hit |= glm::dot(direction, V1) >= 0.0f;

        // find_support_in_direction_of_portal();
        glm::vec3 V4 = minkowskiSupportPoint(body1, transform1.transform, body2, transform2.transform, direction);

        // if (origin outside support plane)
        if (!hit && !isFacing(V4, direction)) {
            // return miss;
            return false;
        }

        // if (Support plane close to portal)
        if (!hit && !isFacing<1e-4f>(V4, direction)) {
            // return miss;
            return false;
        }

        if (hit && glm::dot(direction, V4) - glm::dot(direction, V1) / glm::length(direction) <= 1e-4f) {
            break;
        }

        // choose_new_portal();
        bool inside1 = isFacing(glm::cross(V1 - V0, V4 - V0), V0);
        bool inside2 = isFacing(glm::cross(V2 - V0, V4 - V0), V0);
        bool inside3 = isFacing(glm::cross(V3 - V0, V4 - V0), V0);

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

    glm::vec3 n123 = glm::cross(V1 - V3, V2 - V3);

    contact.normal = -glm::normalize(n123);
    contact.depth = glm::dot(contact.normal, V1);
    contact.point = body1.calcSupportPoint(contact.normal, transform1.transform);

    return true;
}

} // namespace Physbuzz
