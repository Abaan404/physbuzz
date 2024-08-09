#pragma once

#include "objects/builder.hpp"
#include "player.hpp"
#include "renderer.hpp"
#include "ui/handler.hpp"
#include <physbuzz/bindings.hpp>
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

    // object management
    Physbuzz::Scene scene;
    Physbuzz::ResourceManager resources;
    ObjectBuilder builder;

    // game physics
    Physbuzz::Dynamics dynamics;
    Physbuzz::Collision collision;

    // ImGui
    InterfaceManager interface;

    // player controls
    Physbuzz::Bindings bindings;
    Player player;

  private:
    bool m_IsRunning = false;
};
