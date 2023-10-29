#include "2dphysics.hpp"
#include <SDL2/SDL.h>
#include <iostream>

Game::Game() {
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        std::cerr << "SDL_Init Error: " << SDL_GetError() << std::endl;
        exit(1);
    }

    // Create a window
    window = SDL_CreateWindow("SDL2 Example", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 800, 600, SDL_WINDOW_SHOWN);
    if (window == nullptr) {
        std::cerr << "SDL_CreateWindow Error: " << SDL_GetError() << std::endl;
        SDL_Quit();
        exit(1);
    }

    // Create a renderer
    renderer = SDL_CreateRenderer(window, -2, SDL_RENDERER_ACCELERATED);
    if (renderer == nullptr) {
        std::cerr << "SDL_CreateRenderer Error: " << SDL_GetError() << std::endl;
        SDL_DestroyWindow(window);
        SDL_Quit();
        exit(1);
    }

    is_running = true;

    this->painter = new Painter(renderer, &objects);
    this->event_handler = new EventHandler(painter, &objects);
    this->physics = new PhysicsContext(&objects);
}

void Game::game_loop() {
    SDL_Event e;
    while (is_running) {

        // get the next event in queue
        SDL_PollEvent(&e);

        // handle each event, send event to event handler if necessary
        switch (e.type) {
        case SDL_QUIT:
            is_running = false;
            break;

        case SDL_KEYDOWN:
            event_handler->keyboard_keydown(e);
            break;

        case SDL_KEYUP:
            event_handler->keyboard_keyup(e);
            break;

        case SDL_MOUSEBUTTONDOWN:
            event_handler->mouse_mousedown(e);
            break;
        }

        painter->background();
        physics->tick();
        painter->draw();
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
