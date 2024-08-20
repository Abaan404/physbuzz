#pragma once

#include "objects/builder.hpp"
#include "player.hpp"
#include "ui/handler.hpp"
#include <physbuzz/resources/manager.hpp>
#include <physbuzz/window/bindings.hpp>
#include <physbuzz/window/window.hpp>

class Game {
  public:
    Game();
    ~Game();

    void build();
    void destroy();

    void reset();

    void loop();
    const bool &isRunning();

    // displaying and rendering
    Physbuzz::Window window;

    // object management
    Physbuzz::Scene scene;
    ObjectBuilder builder;

    // ImGui
    InterfaceManager interface;

    // player controls
    Physbuzz::Bindings bindings;
    Player player;

  private:
    bool m_IsRunning = false;
};
