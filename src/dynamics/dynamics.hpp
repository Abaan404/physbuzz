#pragma once

#include <glm/glm.hpp>

class DynamicObject {
  public:
    DynamicObject(float mass);
    float mass;
    float intertia = 0.0f;

    glm::vec2 velocity = glm::vec2(0.0f, 0.0f);
    glm::vec2 acceleration = glm::vec2(0.0f, 0.0f);

    virtual void tick() = 0;
};
