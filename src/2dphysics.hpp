#pragma once

#include "events.hpp"
#include "painter.hpp"
#include "physics/physics.hpp"
#include <SDL2/SDL.h>

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
    Physics *physics;

    // list of game objects
    std::list<Object *> objects;

    // SDL Variables
    SDL_Window *window;
    SDL_Renderer *renderer;
};
