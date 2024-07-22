#pragma once

#include "events.hpp"
#include "renderer.hpp"
#include "ui/handler.hpp"
#include "wall.hpp"
#include <physbuzz/clock.hpp>
#include <physbuzz/collision.hpp>
#include <physbuzz/events.hpp>
#include <physbuzz/window.hpp>

class Game {
  public:
    Game();
    ~Game();

    void build();
    void destroy();

    void loop();
    const bool &isRunning();

    // displaying and rendering
    Physbuzz::Window window;
    Renderer renderer;

    // ecs object tracking
    Physbuzz::Scene scene;

    // game physics
    Physbuzz::Dynamics dynamics;
    Physbuzz::Collision collision;

    // ImGui
    InterfaceManager interface;

    // game elements
    Events events;
    Wall wall;

  private:
    bool m_IsRunning = false;
};
