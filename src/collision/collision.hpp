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
        for (const auto &vertex : mesh.vertices) {
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
    float depth = std::numeric_limits<float>::max();
    glm::vec3 normal = {0.0f, 1.0f, 0.0f};
    glm::vec3 point = {0.0f, 0.0f, 0.0f};
};

class Collision {
  public:
    void tick(Physbuzz::Scene &scene);

    bool check(Physbuzz::Object &object1, Physbuzz::Object &object2, Contact &contact);
    void resolve(Physbuzz::Object &object1, Physbuzz::Object &object2, Contact &contact);

  private:
    float getAxisOverlap(const glm::vec3 &axis, const Physbuzz::MeshComponent &mesh1, const Physbuzz::MeshComponent &mesh2);
    void addMeshNormals(const Physbuzz::MeshComponent &mesh, std::vector<glm::vec3> &axes);

    float m_Restitution = 1.0f;
};
