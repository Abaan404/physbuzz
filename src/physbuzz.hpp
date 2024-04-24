#pragma once

#include "dynamics/dynamics.hpp"
#include "events.hpp"
#include "renderer/renderer.hpp"
#include "scene/scene.hpp"
#include "ui/handler.hpp"
#include <memory>

class Game {
  public:
    Game();
    ~Game();

    void game_loop();

  private:
    bool is_running;

    // stuff ig
    std::unique_ptr<EventHandler> event_handler;
    std::unique_ptr<Dynamics> dynamics;
    std::unique_ptr<Renderer> renderer;
    std::unique_ptr<Scene> scene;
    std::unique_ptr<InterfaceHandler> interface;

    // SDL Variables
    SDL_Window *window;
    SDL_GLContext context;
};
