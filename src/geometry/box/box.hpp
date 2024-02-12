#pragma once

#include "../object.hpp"

class Box : public GameObject {
  public:
    Box(glm::vec2 position, float width, float height, float mass);

    float width, height;
    glm::vec2 min, max;
};

class TextureBox : public TextureObject {
  public:
    TextureBox(Box &box, unsigned int &program);

    Box &box;

    void draw(Renderer &renderer, unsigned int usage) override;

  private:
    int u_color;
};

class DynamicBox : public DynamicObject {
  public:
    DynamicBox(Box &box, float mass);

    Box &box;

    void tick() override;
};
