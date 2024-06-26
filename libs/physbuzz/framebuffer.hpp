#pragma once

#include <glad/gl.h>
#include <glm/glm.hpp>

namespace Physbuzz {

class Framebuffer {
  public:
    Framebuffer(glm::ivec2 &resolution);
    ~Framebuffer();

    void build();
    void destroy();

    void bind() const;
    void unbind();

    void resize(glm::ivec2 &resolution);
    void clear(glm::vec4 &color);

    const GLuint getColor() const;
    const GLuint getDepth() const;
    const GLuint getFramebuffer() const;
    const glm::ivec2 getResolution() const;

  private:
    GLuint m_Framebuffer;
    GLuint m_Depth;
    GLuint m_Color;

    glm::ivec2 m_Resolution;
};

} // namespace Physbuzz
