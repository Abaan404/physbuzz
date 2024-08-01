#pragma once

#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/scalar_constants.hpp>

#include <physbuzz/camera.hpp>
#include <physbuzz/clock.hpp>
#include <physbuzz/renderer.hpp>
#include <physbuzz/scene.hpp>

class Renderer {
  public:
    Renderer(Physbuzz::Window *window);
    ~Renderer();

    void build();
    void destroy();

    void render(Physbuzz::Scene &scene);
    void render(Physbuzz::Object &object) const;

    const Physbuzz::Clock &getClock() const;

    Physbuzz::Camera *activeCamera = nullptr;
    Physbuzz::Renderer renderer;

  private:
    Physbuzz::Clock m_Clock;

    glm::vec4 m_ClearColor = glm::vec4(0.0f, 0.0f, 0.0f, 0.0f);
};
