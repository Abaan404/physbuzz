#include "collision.hpp"

#include "../dynamics/dynamics.hpp"
#include <physbuzz/logging.hpp>
#include <physbuzz/renderer.hpp>
#include <vector>

// sweep and prune
void Collision::tick(Physbuzz::Scene &scene) {
    scene.sortObjects([](Physbuzz::Object &object1, Physbuzz::Object &object2) {
        if (!(object1.hasComponent<BoundingComponent>() && object2.hasComponent<BoundingComponent>())) {
            return false;
        }

        const AABBComponent &aabb1 = object1.getComponent<BoundingComponent>().getBox();
        const AABBComponent &aabb2 = object2.getComponent<BoundingComponent>().getBox();

        return aabb1.min.x < aabb2.min.x;
    });

    Contact contact;
    for (auto &object1 : scene.getObjects()) {
        for (auto &object2 : scene.getObjects()) {
            // skip same objects
            if (&object1 == &object2) {
                continue;
            }

            // skip non bounded components
            if (!(object1.hasComponent<BoundingComponent>() && object2.hasComponent<BoundingComponent>())) {
                continue;
            }

            Physbuzz::Logger::ASSERT(object1.hasComponent<Physbuzz::MeshComponent>() && object2.hasComponent<Physbuzz::MeshComponent>(), "Attempting Collision on Non-Mesh Object");
            Physbuzz::Logger::ASSERT(object1.hasComponent<TransformableComponent>() && object2.hasComponent<TransformableComponent>(), "Attempting Collision on Non-Transformable Object");
            Physbuzz::Logger::ASSERT(object1.hasComponent<RigidBodyComponent>() && object2.hasComponent<RigidBodyComponent>(), "Attempting Collision on Non-Physics Object");

            const AABBComponent &aabb1 = object1.getComponent<BoundingComponent>().getBox();
            const AABBComponent &aabb2 = object2.getComponent<BoundingComponent>().getBox();

            if (aabb2.min.x > aabb1.max.x) {
                continue;
            }

            if (check(object1, object2, contact)) {
                resolve(object1, object2, contact);
            }
        }
    }
}

// get the depth of overlap
float Collision::getAxisOverlap(const glm::vec3 &axis, const Physbuzz::MeshComponent &mesh1, const Physbuzz::MeshComponent &mesh2) {
    auto getMinMax = [](const Physbuzz::MeshComponent &mesh, const glm::vec3 &axis) {
        struct {
            float min = std::numeric_limits<float>::max();
            float max = std::numeric_limits<float>::lowest();
        } ret;

        for (const auto &vertex : mesh.vertices) {
            float projection = glm::dot(vertex, axis);

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

void Collision::addMeshNormals(const Physbuzz::MeshComponent &mesh, std::vector<glm::vec3> &axes) {
    static constexpr float PARALLEL_AXIS_THRESHOLD = 1e-3f;

    // TODO do a perf test on n^2 normal check or running on every normal based on no. of vertices
    for (const auto &normal : mesh.normals) {
        bool parallelFound = false;
        for (const auto &axis : axes) {
            if (glm::length(glm::cross(axis, normal)) < PARALLEL_AXIS_THRESHOLD) {
                parallelFound = true;
                break;
            }
        }

        if (parallelFound) {
            continue;
        }

        axes.emplace_back(normal);
    }
}

// Seperating Axis Theorem
// ref:
//  - https://code.tutsplus.com/collision-detection-using-the-separating-axis-theorem--gamedev-169t
//  - https://textbooks.cs.ksu.edu/cis580/04-collisions/04-separating-axis-theorem/
bool Collision::check(Physbuzz::Object &object1, Physbuzz::Object &object2, Contact &contact) {
    std::vector<glm::vec3> axes;
    Physbuzz::MeshComponent &mesh1 = object1.getComponent<Physbuzz::MeshComponent>();
    Physbuzz::MeshComponent &mesh2 = object2.getComponent<Physbuzz::MeshComponent>();

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

void Collision::resolve(Physbuzz::Object &object1, Physbuzz::Object &object2, Contact &contact) {
    TransformableComponent &transform1 = object1.getComponent<TransformableComponent>();
    TransformableComponent &transform2 = object2.getComponent<TransformableComponent>();

    RigidBodyComponent &rigidBody1 = object1.getComponent<RigidBodyComponent>();
    RigidBodyComponent &rigidBody2 = object2.getComponent<RigidBodyComponent>();

    // ensure the normal points from object1 -> object2
    glm::vec3 direction = transform2.position - transform1.position;
    if (glm::dot(contact.normal, direction) < 0.0f) {
        contact.normal = -contact.normal;
    }

    // dont resolve if velocities are separating
    const float velocityAlongNormal = glm::dot(rigidBody2.velocity - rigidBody1.velocity, contact.normal);
    if (velocityAlongNormal > 0) {
        return;
    }

    const float impulseScalar = (-(1 + m_Restitution) * velocityAlongNormal) / ((1 / rigidBody1.mass) + (1 / rigidBody2.mass));
    const glm::vec3 impulse = impulseScalar * contact.normal;

    rigidBody1.velocity -= impulse / rigidBody1.mass;
    rigidBody2.velocity += impulse / rigidBody2.mass;
}
