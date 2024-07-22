#pragma once

#include <physbuzz/clock.hpp>
#include <physbuzz/renderer.hpp>
#include <physbuzz/scene.hpp>

class Renderer {
  public:
    Renderer(Physbuzz::Window &window);
    ~Renderer();

    void build();
    void destroy();

    void render(Physbuzz::Scene &scene);
    void render(Physbuzz::RenderComponent &render);

    const Physbuzz::Clock &getClock() const;
    Physbuzz::Renderer &getRenderer();

  private:
    Physbuzz::Renderer m_Renderer;
    Physbuzz::Clock m_Clock;

    glm::vec4 m_ClearColor = glm::vec4(0.0f, 0.0f, 0.0f, 0.0f);
};
