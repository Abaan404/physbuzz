#include <SDL.h>
#include <cstdio>
#include <iostream>

#include "2dphysics.hpp"

Game::Game() {
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        std::cerr << "SDL_Init Error: " << SDL_GetError() << std::endl;
        exit(1);
    }

    // Create a window
    window =
        SDL_CreateWindow("SDL2 Example", SDL_WINDOWPOS_UNDEFINED,
                         SDL_WINDOWPOS_UNDEFINED, 800, 600, SDL_WINDOW_SHOWN);
    if (window == nullptr) {
        std::cerr << "SDL_CreateWindow Error: " << SDL_GetError() << std::endl;
        SDL_Quit();
        exit(1);
    }

    // Create a renderer
    renderer = SDL_CreateRenderer(window, -2, SDL_RENDERER_ACCELERATED);
    if (renderer == nullptr) {
        std::cerr << "SDL_CreateRenderer Error: " << SDL_GetError()
                  << std::endl;
        SDL_DestroyWindow(window);
        SDL_Quit();
        exit(1);
    }

    is_running = true;

    Painter *painter = new Painter(renderer);
    context.painter = painter;
    context.event_handler = new EventHandler(painter);
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
            context.event_handler->keyboard_keydown(e);
            break;

        case SDL_KEYUP:
            context.event_handler->keyboard_keyup(e);
            break;

        case SDL_MOUSEBUTTONDOWN:
            context.event_handler->mouse_mousedown(e);
            break;
        }

        context.painter->background();
        context.painter->draw();
    }

    cleanup();
}

void Game::cleanup() {
    free(context.painter);
    free(context.event_handler);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}

int main(int argc, char *argv[]) {
    Game game = Game();
    game.game_loop();

    return 0;
}
