#pragma once

#include "ui/handler.hpp"
#include "wall.hpp"
#include <physbuzz/clock.hpp>
#include <physbuzz/collision.hpp>
#include <physbuzz/events.hpp>
#include <physbuzz/renderer.hpp>
#include <physbuzz/window.hpp>

class Game {
  public:
    Game();
    ~Game();

    void loop();
    bool isRunning();

    // displaying and rendering
    Physbuzz::Window window;
    Physbuzz::Renderer renderer;
    Physbuzz::Clock clock;

    // ecs object tracking
    Physbuzz::Scene scene;

    // input handlers
    Physbuzz::EventManager eventManager;

    // ImGui UI builder
    InterfaceManager interface;

    // game physics
    Physbuzz::Dynamics dynamics;
    Physbuzz::Collision collision;

    // game elements
    Wall wall;

  private:
    bool m_IsRunning = false;
};
