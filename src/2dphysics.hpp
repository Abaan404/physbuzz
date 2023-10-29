#pragma once

#include "events.hpp"
#include "painter.hpp"
#include "physics.hpp"
#include <SDL2/SDL.h>
#include <vector>

class Game {
  public:
    Game();
    void game_loop();
    void cleanup();

  private:
    bool is_running;

    // stuff ig
    EventHandler *event_handler;
    Painter *painter;
    PhysicsContext *physics;

    // holds all game objects
    std::vector<GameObject *> objects;

    // SDL Variables
    SDL_Window *window;
    SDL_Renderer *renderer;
};
