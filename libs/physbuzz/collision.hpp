#pragma once

#include "events.hpp"
#include <glm/glm.hpp>
#include <list>
#include <memory>
#include <physbuzz/mesh.hpp>
#include <physbuzz/scene.hpp>

namespace Physbuzz {

struct AABBComponent {
    glm::vec3 min = glm::vec3(std::numeric_limits<float>::max());
    glm::vec3 max = glm::vec3(std::numeric_limits<float>::lowest());
};

struct BoundingComponent {
    BoundingComponent(const AABBComponent &quad);
    BoundingComponent(const Mesh &mesh);

    AABBComponent aabb;
};

struct Contact {
    ObjectID object1;
    ObjectID object2;

    float depth = std::numeric_limits<float>::max();
    glm::vec3 normal = {0.0f, 1.0f, 0.0f};

    glm::vec3 point1 = {0.0f, 0.0f, 0.0f};
    glm::vec3 point2 = {0.0f, 0.0f, 0.0f};
};

class ICollisionDetector {
  public:
    ICollisionDetector(Scene &scene);
    virtual ~ICollisionDetector() = default;

    virtual void build();
    virtual void destroy();

    virtual bool check(Contact &contact) = 0;
    virtual std::list<Contact> find();
    virtual void find(std::list<Contact> &contacts);

  protected:
    Scene &m_Scene;
};

class ICollisionResolver : public IEventSubject {
  public:
    ICollisionResolver(Scene &scene);

    virtual void build();
    virtual void destroy();

    virtual ~ICollisionResolver() = default;
    virtual void solve(std::list<Contact> &contacts);
    virtual void solve(const Contact &contact) = 0;

  protected:
    Scene &m_Scene;
};

class Collision {
  public:
    Collision(std::shared_ptr<ICollisionDetector> narrow, std::shared_ptr<ICollisionDetector> broad, std::shared_ptr<ICollisionResolver> resolver);
    ~Collision();

    void build();
    void destroy();

    void tick(Scene &scene);

  protected:
    std::shared_ptr<ICollisionDetector> narrowDetector;
    std::shared_ptr<ICollisionDetector> broadDetector;

    std::shared_ptr<ICollisionResolver> resolver;
};

} // namespace Physbuzz
