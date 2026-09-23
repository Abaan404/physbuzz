#include "sepaxis.hpp"

#include "../../ecs/scene.hpp"
#include "../components/rigidbody.hpp"

namespace Physbuzz {

SeperatingAxis2D::SeperatingAxis2D(Scene *scene)
    : CollisionDetector(scene) {}

bool SeperatingAxis2D::buildImpl() {
    return true;
}

bool SeperatingAxis2D::destroyImpl() {
    return true;
}

bool SeperatingAxis2D::checkImpl(Contact &contact) {
    const auto [body1] = m_Scene->getComponent<RigidBodyComponent>(contact.object1);
    const auto [body2] = m_Scene->getComponent<RigidBodyComponent>(contact.object2);

    std::vector<glm::vec3> axes;

    addMeshNormals(body1, axes);
    addMeshNormals(body2, axes);

    for (const auto &axis : axes) {
        float axisOverlap = getAxisOverlap(axis, body1, body2);

        if (axisOverlap <= 0.0f) {
            return false;
        }

        if (axisOverlap < contact.depth) {
            contact.depth = axisOverlap;
            contact.normal = axis;
        }
    }

    return true;
}

// get the depth of overlap
float SeperatingAxis2D::getAxisOverlap(const glm::vec3 &axis, const RigidBodyComponent &body1, const RigidBodyComponent &body2) {
    auto getMinMax = [](const RigidBodyComponent &body, const glm::vec3 &axis) {
        struct {
            float min = std::numeric_limits<float>::max();
            float max = std::numeric_limits<float>::lowest();
        } ret;

        for (const auto &vertex : body.getInfo().mesh->geometry.vertices) {
            float projection = glm::dot(vertex.position, axis);

            ret.min = glm::min(projection, ret.min);
            ret.max = glm::max(projection, ret.max);
        }

        return ret;
    };

    const auto proj1 = getMinMax(body1, axis);
    const auto proj2 = getMinMax(body2, axis);

    if (proj1.max < proj2.min || proj1.min > proj2.max) {
        return 0.0f;
    }

    const float overlap1 = proj1.max - proj2.min;
    const float overlap2 = proj2.max - proj1.min;

    return glm::min(overlap1, overlap2);
}

void SeperatingAxis2D::addMeshNormals(const RigidBodyComponent &body, std::vector<glm::vec3> &axes) {
    for (const auto &vertex : body.getInfo().mesh->geometry.vertices) {
        axes.emplace_back(vertex.normal);
    }
}

} // namespace Physbuzz
