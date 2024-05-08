#pragma once

#include <glm/glm.hpp>
#include <physbuzz/object.hpp>
#include <physbuzz/scene.hpp>

struct AABBComponent {
    glm::vec3 min;
    glm::vec3 max;
};

struct Contact {
    glm::vec3 normal;
    glm::vec3 tangent;
    float depth;
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
