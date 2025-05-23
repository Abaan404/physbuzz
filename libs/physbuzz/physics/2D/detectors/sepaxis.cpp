#include "sepaxis.hpp"

namespace Physbuzz {

SeperatingAxis2D::SeperatingAxis2D(Scene *scene)
    : ICollisionDetector(scene) {}

bool SeperatingAxis2D::check(Contact &contact) {
    if (!(m_Scene->containsComponent<Mesh>(contact.object1) && m_Scene->containsComponent<Mesh>(contact.object2))) {
        return false;
    }

    const Mesh &mesh1 = m_Scene->getComponent<Mesh>(contact.object1);
    const Mesh &mesh2 = m_Scene->getComponent<Mesh>(contact.object2);

    std::vector<glm::vec3> axes;

    addMeshNormals(mesh1, axes);
    addMeshNormals(mesh2, axes);

    for (const auto &axis : axes) {
        float axisOverlap = getAxisOverlap(axis, mesh1, mesh2);

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
float SeperatingAxis2D::getAxisOverlap(const glm::vec3 &axis, const Mesh &mesh1, const Mesh &mesh2) {
    auto getMinMax = [](const Mesh &mesh, const glm::vec3 &axis) {
        struct {
            float min = std::numeric_limits<float>::max();
            float max = std::numeric_limits<float>::lowest();
        } ret;

        for (const auto &vertex : mesh.vertices) {
            float projection = glm::dot(vertex.position, axis);

            ret.min = glm::min(projection, ret.min);
            ret.max = glm::max(projection, ret.max);
        }

        return ret;
    };

    // auto is truly cpp magic
    const auto proj1 = getMinMax(mesh1, axis);
    const auto proj2 = getMinMax(mesh2, axis);

    if (proj1.max < proj2.min || proj1.min > proj2.max) {
        return 0.0f;
    }

    const float overlap1 = proj1.max - proj2.min;
    const float overlap2 = proj2.max - proj1.min;

    return glm::min(overlap1, overlap2);
}

void SeperatingAxis2D::addMeshNormals(const Mesh &mesh, std::vector<glm::vec3> &axes) {
    constexpr float PARALLEL_AXIS_THRESHOLD = 1e-3f;

    // TODO do a perf test on n^2 normal check or running on every normal based on no. of vertices
    for (const auto &vertex : mesh.vertices) {
        bool parallelFound = false;
        for (const auto &axis : axes) {
            if (glm::length(glm::cross(axis, vertex.normal)) < PARALLEL_AXIS_THRESHOLD) {
                parallelFound = true;
                break;
            }
        }

        if (parallelFound) {
            continue;
        }

        axes.emplace_back(vertex.normal);
    }
}

} // namespace Physbuzz
