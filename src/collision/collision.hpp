#pragma once

#include <glm/glm.hpp>
#include <physbuzz/object.hpp>
#include <physbuzz/scene.hpp>

struct AABBComponent {
    glm::vec3 min = {0.0f, 0.0f, 0.0f};
    glm::vec3 max = {0.0f, 0.0f, 0.0f};

    // build using mesh if available
    static AABBComponent buildWithVertex(std::vector<glm::vec3> &vertices) {
        // Initialize min and max with the first vertex
        AABBComponent aabb = {
            .min = {0.0f, 0.0f, 0.0f},
            .max = {0.0f, 0.0f, 0.0f},
        };

        for (const auto &vertex : vertices) {
            aabb.min.x = std::min(aabb.min.x, vertex.x);
            aabb.min.y = std::min(aabb.min.y, vertex.y);
            aabb.min.z = std::min(aabb.min.z, vertex.z);

            aabb.max.x = std::max(aabb.max.x, vertex.x);
            aabb.max.y = std::max(aabb.max.y, vertex.y);
            aabb.max.z = std::max(aabb.max.z, vertex.z);
        }

        return aabb;
    }
};

struct Contact {
    glm::vec3 normal = {0.0f, 1.0f, 0.0f};
    float depth = 0.0f;
};

class Collision {
  public:
    void tick(Physbuzz::Scene &scene);

  private:
    bool check(Physbuzz::Object &object1, Physbuzz::Object &object2);
    void resolve(Physbuzz::Object &object1, Physbuzz::Object &object2);

    Contact calcContact(Physbuzz::Object &object1, Physbuzz::Object &object2);

    float m_Restitution{0.5f};
};
