#pragma once

#include "events.hpp"
#include "renderer/renderer.hpp"
#include "scene.hpp"
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
    std::unique_ptr<Renderer> renderer;
    std::unique_ptr<SceneManager> scene_manager;
    std::unique_ptr<InterfaceHandler> interface;

    // holds all game objects
    std::vector<std::shared_ptr<GameObject>> objects;

    // SDL Variables
    SDL_Window *window;

    // OpenGL Variables
    SDL_GLContext context;
};
