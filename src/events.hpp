#pragma once

#include "renderer/renderer.hpp"
#include "ui/handler.hpp"

#include <SDL_events.h>

class EventHandler {
  public:
    EventHandler(Renderer &renderer, InterfaceHandler &interface, Scene &scene) : renderer(renderer), interface(interface), scene(scene){};

    // execute all event in queue
    void execute();

    // sdl2 events
    void keyboard_keyup(SDL_KeyboardEvent &event);
    void keyboard_keydown(SDL_KeyboardEvent &event);
    void mouse_mousedown(SDL_MouseButtonEvent &event);
    void mouse_mouseup(SDL_MouseButtonEvent &event);
    void mouse_mousescroll(SDL_MouseButtonEvent &event);
    void mouse_mousemotion(SDL_MouseButtonEvent &event);
    void video_resize(SDL_Event &event);

  private:
    Renderer &renderer;
    InterfaceHandler &interface;
    Scene &scene;
};
