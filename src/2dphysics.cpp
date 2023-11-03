#include "2dphysics.hpp"
#include <SDL2/SDL.h>
#include <cstdio>

#include <imgui_impl_sdl2.h>
#include <imgui_impl_sdlrenderer2.h>
#include <imgui.h>

Game::Game() {
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_GAMECONTROLLER) != 0) {
        printf("[ERROR] SDL_Init: %s\n", SDL_GetError());
        exit(1);
    }

    // Create a window
    SDL_WindowFlags window_flags = (SDL_WindowFlags)(SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);
    window = SDL_CreateWindow("SDL2 Example", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 800, 600, window_flags);
    if (window == nullptr) {
        printf("[ERROR] SDL_CreateWindow: %s\n", SDL_GetError());
        SDL_Quit();
        exit(1);
    }

    // Create a renderer
    SDL_RendererFlags renderer_flags = (SDL_RendererFlags)(SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    renderer = SDL_CreateRenderer(window, -2, renderer_flags);
    if (renderer == nullptr) {
        printf("[ERROR] SDL_CreateRenderer: %s\n", SDL_GetError());
        SDL_Quit();
        exit(1);
    }

    // Setup ImGui
    ImGui::CreateContext();

    ImGuiIO &io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;  // Enable Gamepad Controls

    ImGui::StyleColorsDark();

    ImGui_ImplSDL2_InitForSDLRenderer(window, renderer);
    ImGui_ImplSDLRenderer2_Init(renderer);

    is_running = true;

    this->painter = new Painter(renderer, &objects);
    this->interface = new UserInferface(*painter);
    this->event_handler = new EventHandler(painter, interface, &objects);
    this->physics = new PhysicsContext(&objects);
}

void Game::game_loop() {
    SDL_Event event;
    while (is_running) {

        // get the next event in queue
        SDL_PollEvent(&event);
        
        // pass event to imgui
        ImGui_ImplSDL2_ProcessEvent(&event);

        // handle each event, send event to event handler if necessary
        switch (event.type) {
        case SDL_QUIT:
            is_running = false;
            break;

        case SDL_KEYDOWN:
            event_handler->keyboard_keydown(event.key);
            break;

        case SDL_KEYUP:
            event_handler->keyboard_keyup(event.key);
            break;

        case SDL_MOUSEBUTTONDOWN:
            event_handler->mouse_mousedown(event.button);
            break;
        }

        interface->draw();
        physics->tick();
        painter->render();
    }
}

void Game::cleanup() {
    free(painter);
    free(event_handler);
    free(physics);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}

int main(int argc, char *argv[]) {
    Game game = Game();
    game.game_loop();
    game.cleanup();

    return 0;
}
