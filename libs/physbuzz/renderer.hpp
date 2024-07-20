#pragma once

#include "clock.hpp"
#include "framebuffer.hpp"
#include "mesh.hpp"
#include "scene.hpp"
#include "shaders.hpp"
#include "window.hpp"
#include <glad/gl.h>

namespace Physbuzz {

class RenderComponent {
  public:
    RenderComponent(const Mesh &mesh, const ShaderPipeline &shader);
    ~RenderComponent();

    void build();
    void destroy();

    void bind() const;
    void unbind() const;

    void draw() const;

    // TODO uniforms class abstraction
    GLint gluTime;
    GLint gluTimedelta;
    GLint gluResolution;

    Mesh mesh;
    ShaderPipeline pipeline;
};

class Renderer {
  public:
    Renderer(Window &window);
    Renderer(const Renderer &other);
    Renderer &operator=(const Renderer &other);
    ~Renderer();

    void build();
    void destroy();

    void render(Scene &scene);
    void render(RenderComponent &render);

    void target(Framebuffer *framebuffer);
    void clear(const glm::vec4 &color);
    void resize(const glm::ivec2 &resolution);

    const Window &getWindow() const;
    const glm::ivec2 &getResolution() const;

    void normalize(Mesh &mesh);

  private:
    void copy(const Renderer &other);

    Framebuffer *m_Framebuffer;
    Window &m_Window;
    Clock m_Clock;
    glm::ivec2 m_Resolution;
};

} // namespace Physbuzz
