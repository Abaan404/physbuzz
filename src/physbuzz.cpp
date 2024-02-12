#include "physbuzz.hpp"

#include "debug.hpp"

#include <SDL2/SDL.h>
#include <cstdio>
#include <glad/gl.h>
#include <imgui.h>
#include <imgui_impl_opengl3.h>
#include <imgui_impl_sdl2.h>
#include <memory>

Game::Game() {
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_GAMECONTROLLER) != 0) {
        printf("[ERROR] SDL_Init: %s\n", SDL_GetError());
        exit(1);
    }

    // Create a window
    SDL_WindowFlags window_flags = (SDL_WindowFlags)(SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI | SDL_WINDOW_OPENGL);
    window = SDL_CreateWindow("phyzbuzz engine", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 1280, 720, window_flags);
    if (window == nullptr) {
        printf("[ERROR] SDL_CreateWindow: %s\n", SDL_GetError());
        SDL_Quit();
        exit(1);
    }

    // Setup OpenGL
    SDL_GL_LoadLibrary(NULL);
    context = SDL_GL_CreateContext(window);
    if (context == NULL) {
        printf("[ERROR] SDL_GL_CreateContext: Failed to create OpenGL context\n");
        SDL_Quit();
        exit(1);
    }

    int version = gladLoadGL((GLADloadfunc)SDL_GL_GetProcAddress);

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 6);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_DEBUG_FLAG);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

    SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);

    SDL_GL_MakeCurrent(window, context);
    SDL_GL_SetSwapInterval(1); // Enable vsync

    // debug context setup
    glEnable(GL_DEBUG_OUTPUT);
    glDebugMessageCallback(OpenGLDebugCallback, 0);

    int w, h;
    SDL_GetWindowSize(window, &w, &h);
    glViewport(0, 0, w, h);
    glClearColor(0.0f, 0.5f, 1.0f, 0.0f);

    is_running = true;

    // create ImGui context
    // (this doesnt work if its called in the UserInferface
    // constructor for some reason
    ImGui::CreateContext();

    // there has to be a better way to pass by reference
    this->renderer = std::make_unique<Renderer>(&context, window, objects);
    this->interface = std::make_unique<UserInferface>(*renderer);
    this->event_handler = std::make_unique<EventHandler>(*renderer, *interface, objects);
    this->scene_manager = std::make_unique<SceneManager>(objects);
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

        interface->render();
        renderer->render();
        scene_manager->tick();
    }
}

Game::~Game() {
    SDL_GL_DeleteContext(context);
    SDL_DestroyWindow(window);
    SDL_Quit();
}

int main(int argc, char *argv[]) {
    Game game = Game();
    game.game_loop();

    return 0;
}
