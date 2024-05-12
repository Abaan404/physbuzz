#pragma once

#include <glm/glm.hpp>
#include <physbuzz/mesh.hpp>
#include <physbuzz/scene.hpp>

struct AABBComponent {
    glm::vec3 min = {0.0f, 0.0f, 0.0f};
    glm::vec3 max = {0.0f, 0.0f, 0.0f};
};

class BoundingComponent {
  public:
    BoundingComponent(const AABBComponent &quad) {
        build(quad);
    }
    BoundingComponent(const Physbuzz::MeshComponent &mesh) {
        build(mesh);
    }

    void build(const AABBComponent &quad) {
        aabb = quad;
    }

    void build(const Physbuzz::MeshComponent &mesh) {
        for (const auto &vertex : mesh.screenVertices) {
            aabb.min = glm::min(aabb.min, vertex);
            aabb.max = glm::max(aabb.max, vertex);
        }
    }

    const AABBComponent &getBox() {
        return aabb;
    }

  private:
    AABBComponent aabb;
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

    float m_Restitution{1.f};
};
