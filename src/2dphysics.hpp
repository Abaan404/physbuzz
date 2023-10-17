#pragma once

#include "events.hpp"
#include "painter.hpp"
#include <SDL2/SDL.h>

struct GameContext {
    EventHandler *event_handler;
    Painter *painter;
};

class Game {
  public:
    Game();
    void game_loop();
    void cleanup();

    bool is_running;

    // SDL Variables
    SDL_Window *window;
    SDL_Renderer *renderer;

    GameContext context;
};
