#pragma once

#include "../ecs/scene.hpp"
#include "../events/handler.hpp"
#include "../render/mesh.hpp"
#include "../render/model.hpp"
#include <list>

namespace Physbuzz {

struct AABBComponent {
    AABBComponent(const Mesh &mesh, const TransformComponent &transform);
    AABBComponent(const std::vector<Mesh> &meshes, const TransformComponent &transform);

    glm::vec3 min = glm::vec3(std::numeric_limits<float>::max());
    glm::vec3 max = glm::vec3(std::numeric_limits<float>::lowest());
};

struct Contact {
    ObjectID object1;
    ObjectID object2;

    float depth = std::numeric_limits<float>::max();
    glm::vec3 normal = {0.0f, 1.0f, 0.0f};

    glm::vec3 point1 = {0.0f, 0.0f, 0.0f};
    glm::vec3 point2 = {0.0f, 0.0f, 0.0f};
};

class ICollisionDetector : public EventSubject {
  public:
    ICollisionDetector(Scene *scene);
    virtual ~ICollisionDetector() = default;

    virtual void build();
    virtual void destroy();

    virtual bool check(Contact &contact) = 0;
    virtual std::list<Contact> find();
    virtual void find(std::list<Contact> &contacts);

  protected:
    Scene *m_Scene;
};

class ICollisionResolver : public EventSubject {
  public:
    ICollisionResolver(Scene *scene);
    virtual ~ICollisionResolver() = default;

    virtual void build();
    virtual void destroy();

    virtual void solve(std::list<Contact> &contacts);
    virtual void solve(const Contact &contact) = 0;

  protected:
    Scene *m_Scene;
};

} // namespace Physbuzz
