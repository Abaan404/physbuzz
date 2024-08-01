#pragma once

#include "framebuffer.hpp"
#include "mesh.hpp"
#include "shaders.hpp"
#include "texture.hpp"
#include "window.hpp"

namespace Physbuzz {

class RenderComponent {
  public:
    RenderComponent(const Mesh &mesh, const ShaderPipeline &pipeline, const Texture &texture);
    ~RenderComponent();

    void build();
    void destroy();

    void bind() const;
    void unbind() const;

    void draw() const;

    Mesh mesh;
    ShaderPipeline pipeline;
    Texture texture;
};

class Renderer {
  public:
    Renderer(Window *window);
    Renderer(const Renderer &other);
    Renderer &operator=(const Renderer &other);
    ~Renderer();

    void build();
    void destroy();

    void render(const RenderComponent &render) const;

    void target(Framebuffer *framebuffer);
    void clear(const glm::vec4 &color);
    void resize(const glm::ivec2 &resolution);

    const Window *getWindow() const;
    const glm::ivec2 &getResolution() const;

  private:
    void copy(const Renderer &other);

    Framebuffer *m_Framebuffer = nullptr;
    Window *m_Window = nullptr;

    glm::ivec2 m_Resolution;
};

} // namespace Physbuzz
