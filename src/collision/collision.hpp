#pragma once

#include <physbuzz/object.hpp>
#include <physbuzz/scene.hpp>
#include <glm/glm.hpp>

struct AABBComponent {
    glm::vec3 min;
    glm::vec3 max;
};

struct DisplacementOutput {
    glm::vec3 normal;
    glm::vec3 tangent;
    float depth;
};

struct ImpulseOutput {
    glm::vec3 normal;
    glm::vec3 tangent;
    float mass;
};

class Collision {
  public:
    void tick(Physbuzz::Scene &scene);

  private:
    bool check(Physbuzz::Object &object1, Physbuzz::Object &object2);
    void resolve(Physbuzz::Object &object1, Physbuzz::Object &object2);

    DisplacementOutput calcDisplacement(Physbuzz::Object &object1, Physbuzz::Object &object2);
    ImpulseOutput calcImpulse(Physbuzz::Object &object1, Physbuzz::Object &object2, DisplacementOutput &displacement);
    void updateMesh(Physbuzz::Object &object1, Physbuzz::Object &object2, glm::vec3 &displacement);
};
