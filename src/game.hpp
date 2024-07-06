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
    Physbuzz::WindowEvents events;
    Physbuzz::Renderer renderer;

    // ecs object tracking
    Physbuzz::Scene scene;

    // ImGui UI builder
    InterfaceManager interface;

    // game physics
    Physbuzz::Clock clock;
    Physbuzz::Dynamics dynamics;
    Physbuzz::Collision collision;

    // game elements
    Wall wall;

  private:
    bool m_IsRunning = false;
};
