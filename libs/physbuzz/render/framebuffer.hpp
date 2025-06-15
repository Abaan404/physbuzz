#pragma once

#include <glad/gl.h>
#include <glm/glm.hpp>

namespace Physbuzz {

struct FramebufferInfo {
    glm::ivec2 resolution = {1280, 720};
    glm::vec4 colorClear = {0.0f, 0.0f, 0.0f, 0.0f};
};

class Framebuffer {
  public:
    Framebuffer(const FramebufferInfo &info);

    void build();
    void destroy();

    void bind() const;
    void unbind() const;

    void resize(const glm::ivec2 &resolution);
    void clear();

    GLuint getColor() const;
    GLuint getDepth() const;
    GLuint getFramebuffer() const;
    const glm::ivec2 &getResolution() const;

  private:
    GLuint m_Framebuffer;
    GLuint m_Depth;
    GLuint m_Color;

    FramebufferInfo m_Info;
};

} // namespace Physbuzz
