#pragma once

#include "ui/handler.hpp"
#include <physbuzz/ecs/scene.hpp>
#include <physbuzz/window/window.hpp>

class Game {
  public:
    void build();
    void destroy();

    void rebuild();

    void loop();
    const bool &isRunning();

    // displaying and rendering
    Physbuzz::Window window;

    // object management
    Physbuzz::Scene scene;

    // ImGui
    InterfaceManager interface;

  private:
    bool m_IsRunning = false;
};
