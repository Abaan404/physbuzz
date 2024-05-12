#pragma once

#include <glm/glm.hpp>

namespace Physbuzz {

class Framebuffer {
  public:
    Framebuffer(glm::ivec2 &resolution);
    Framebuffer(const Framebuffer &other);
    Framebuffer &operator=(const Framebuffer &other);
    ~Framebuffer();

    void build();
    void destroy();

    void bind() const;
    static void unbind();

    void resize(glm::ivec2 &resolution);
    static void clear(glm::vec4 &color);

    unsigned int getColor() const;
    unsigned int getDepth() const;
    unsigned int getFramebuffer() const;
    glm::ivec2 getResolution() const;

  private:
    unsigned int m_Framebuffer;
    unsigned int m_Depth;
    unsigned int m_Color;

    glm::ivec2 m_Resolution;
};

} // namespace Physbuzz
