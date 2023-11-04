#pragma once

#include "events.hpp"
#include "painter.hpp"
#include "physics.hpp"
#include "ui.hpp"

#include <SDL2/SDL.h>
#include <cstdio>
#include <imgui.h>
#include <imgui_impl_sdl2.h>
#include <imgui_impl_sdlrenderer2.h>
#include <memory>
#include <vector>

class Game {
  public:
    Game();
    void game_loop();
    void cleanup();

  private:
    bool is_running;

    // stuff ig
    std::unique_ptr<EventHandler> event_handler;
    std::unique_ptr<Painter> painter;
    std::unique_ptr<PhysicsContext> physics;
    std::unique_ptr<UserInferface> interface;

    // holds all game objects
    std::vector<std::shared_ptr<GameObject>> objects;

    // SDL Variables
    SDL_Window *window;
    SDL_Renderer *renderer;
};
