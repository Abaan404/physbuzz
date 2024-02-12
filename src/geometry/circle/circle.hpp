#pragma once

#include "../object.hpp"

class Circle : public GameObject {
  public:
    Circle(glm::vec2 position, float radius, float mass);

    float radius;
};

class TextureCircle : public TextureObject {
  public:
    TextureCircle(Circle &circle, unsigned int &program);

    Circle &circle;

    void draw(Renderer &renderer, unsigned int usage) override;

  private:
    int gl_color;
};
