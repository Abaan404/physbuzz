#pragma once

#include "events.hpp"
#include "painter.hpp"
#include "scene.hpp"
#include "ui.hpp"
#include "vulkan.hpp"

#include <SDL2/SDL.h>
#include <cstdio>
#include <imgui.h>
#include <imgui_impl_sdl2.h>
#include <imgui_impl_vulkan.h>
#include <memory>
#include <vector>
#include <vulkan/vulkan.h>

class Game {
  public:
    Game();
    void game_loop();
    void cleanup(int exit_code);

  private:
    bool is_running = true;

    bool is_sdl_init = false;
    bool is_vulkan_init = false;
    bool is_imgui_init = false;

    // stuff ig
    std::unique_ptr<EventHandler> event_handler;
    std::unique_ptr<Painter> painter;
    std::unique_ptr<SceneManager> scene_manager;
    std::unique_ptr<UserInferface> interface;

    // holds all game objects
    std::vector<std::shared_ptr<GameObject>> objects;

    // SDL Variables
    SDL_Window *window;

    // Vulkan Variables
    VulkanContext vk_context;
};
