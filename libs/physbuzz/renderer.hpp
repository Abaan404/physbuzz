#pragma once

#include "framebuffer.hpp"
#include "mesh.hpp"
#include "scene.hpp"
#include "window.hpp"
#include <glad/gl.h>

namespace Physbuzz {

class Renderer {
  public:
    Renderer(Window &window);
    Renderer(const Renderer &other);
    Renderer &operator=(const Renderer &other);
    ~Renderer();

    std::time_t time = 0;

    void build();
    void destroy();

    void render(Scene &scene);
    void render(Object &object);

    void target(Framebuffer *framebuffer);
    void clear(glm::vec4 &color);
    void resize(glm::ivec2 &resolution);

    Window &getWindow() const;
    glm::ivec2 getResolution() const;

  private:
    Framebuffer *m_Framebuffer;
    Window &m_Window;
    glm::ivec2 m_Resolution;

    void normalize(MeshComponent &mesh);
};

} // namespace Physbuzz
