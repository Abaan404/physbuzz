#pragma once

#include <glm/glm.hpp>

class Framebuffer {
  public:
    Framebuffer(glm::ivec2 &resolution);
    ~Framebuffer();

    glm::ivec2 resolution;

    void bind();
    static void unbind();

    void resize(glm::ivec2 &resolution);
    static void clear(glm::vec4 &color);

    unsigned int get_color_id() { return color; };

  private:
    unsigned int framebuffer;
    unsigned int depth;
    unsigned int color;
};
