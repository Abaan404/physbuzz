#pragma once

#include "events.hpp"
#include "objects/builder.hpp"
#include "renderer.hpp"
#include "ui/handler.hpp"
#include <physbuzz/clock.hpp>
#include <physbuzz/collision.hpp>
#include <physbuzz/dynamics.hpp>
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
    ObjectBuilder builder;

  private:
    bool m_IsRunning = false;
};
