#pragma once

#include <glm/glm.hpp>
#include <list>
#include <memory>
#include <physbuzz/mesh.hpp>
#include <physbuzz/scene.hpp>

namespace Physbuzz {

struct AABBComponent {
    glm::vec3 min = {0.0f, 0.0f, 0.0f};
    glm::vec3 max = {0.0f, 0.0f, 0.0f};
};

class BoundingComponent {
  public:
    BoundingComponent(const AABBComponent &quad) {
        build(quad);
    }
    BoundingComponent(const MeshComponent &mesh) {
        build(mesh);
    }

    void build(const AABBComponent &quad) {
        aabb = quad;
    }

    void build(const MeshComponent &mesh) {
        for (const auto &vertex : mesh.vertices) {
            aabb.min = glm::min(aabb.min, vertex);
            aabb.max = glm::max(aabb.max, vertex);
        }
    }

    const AABBComponent &getBox() const {
        return aabb;
    }

  private:
    AABBComponent aabb;
};

struct Contact {
    ObjectID object1;
    ObjectID object2;

    float depth = std::numeric_limits<float>::max();
    glm::vec3 normal = {0.0f, 1.0f, 0.0f};
    glm::vec3 point = {0.0f, 0.0f, 0.0f};
};

class ICollisionDetector {
  public:
    virtual ~ICollisionDetector() = default;
    virtual bool check(Scene &scene, Contact &contact) = 0;

    virtual std::list<Contact> find(Scene &scene);
    virtual void find(Scene &scene, std::list<Contact> &contacts);
    virtual void reset();
};

class ICollisionResolver {
  public:
    virtual ~ICollisionResolver() = default;
    virtual void solve(Scene &scene, std::list<Contact> &contacts);
    virtual void solve(Scene &scene, const Contact &contact) = 0;

    virtual void reset();
};

class Collision {
  public:
    Collision(std::shared_ptr<ICollisionDetector> narrow, std::shared_ptr<ICollisionDetector> broad, std::shared_ptr<ICollisionResolver> resolver);
    ~Collision();

    void tick(Scene &scene);

  protected:
    std::shared_ptr<ICollisionDetector> narrowDetector;
    std::shared_ptr<ICollisionDetector> broadDetector;

    std::shared_ptr<ICollisionResolver> resolver;
};

} // namespace Physbuzz
