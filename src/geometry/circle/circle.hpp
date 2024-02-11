#pragma once

#include "../object.hpp"

class Circle : public GameObject {
  public:
    Circle(glm::vec2 position, float radius, float mass);

    float radius;

    void draw(Renderer *renderer, unsigned int usage) override;

  private:
    int gl_color;
};
