#pragma once

#include <physbuzz/camera.hpp>
#include <physbuzz/clock.hpp>
#include <physbuzz/framebuffer.hpp>
#include <physbuzz/mesh.hpp>
#include <physbuzz/resources.hpp>
#include <physbuzz/scene.hpp>
#include <physbuzz/window.hpp>

class Renderer {
  public:
    Renderer(Physbuzz::Window *window);
    ~Renderer();

    void build();
    void destroy();

    void target(Physbuzz::Framebuffer *framebuffer);
    void render(const Physbuzz::Scene &scene) const;

    void clear(const glm::vec4 &color);
    void resize(const glm::ivec2 &resolution);

    void render(Physbuzz::Scene &scene, Physbuzz::ResourceManager &resources);
    void render(Physbuzz::Object &object, Physbuzz::ResourceManager &resources) const;

    const Physbuzz::Clock &getClock() const;
    const glm::ivec2 &getResolution() const;

    Physbuzz::Camera *activeCamera = nullptr;

  private:
    Physbuzz::Clock m_Clock;

    glm::ivec2 m_Resolution;
    glm::vec4 m_ClearColor = glm::vec4(0.0f, 0.0f, 0.0f, 0.0f);

    // FIXME: target only FBOs, remove dependency on a "window"
    Physbuzz::Framebuffer *m_Framebuffer = nullptr;
    Physbuzz::Window *m_Window = nullptr;
};
