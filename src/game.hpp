#pragma once

#include <physbuzz/ecs/scene.hpp>
#include <physbuzz/window/window.hpp>

class Game {
  public:
    void build();
    void destroy();

    void rebuild();

    void loop();
    const bool &isRunning();

    Physbuzz::Window window;
    Physbuzz::Scene scene;

  private:
    bool m_IsRunning = false;
};
