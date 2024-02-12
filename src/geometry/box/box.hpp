#pragma once

#include "../object.hpp"

class Box : public GameObject {
  public:
    Box(glm::vec2 position, float width, float height, float mass);

    glm::vec2 min;
    glm::vec2 max;
};

class TextureBox : public TextureObject {
  public:
    TextureBox(Box &box, unsigned int &program);

    Box &box;

    void draw(Renderer &renderer, unsigned int usage) override;

  private:
    int u_color;
};
