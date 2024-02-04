#pragma once

#include "events.hpp"
#include "renderer.hpp"
#include "scene.hpp"
#include "ui.hpp"
#include "opengl/debug.hpp"

#include <SDL2/SDL.h>
#include <glad/gl.h>
#include <imgui.h>
#include <imgui_impl_opengl3.h>
#include <imgui_impl_sdl2.h>
#include <memory>
#include <vector>

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
    std::unique_ptr<UserInferface> interface;

    // holds all game objects
    std::vector<std::shared_ptr<GameObject>> objects;

    // SDL Variables
    SDL_Window *window;

    // OpenGL Variables
    SDL_GLContext context;
};
