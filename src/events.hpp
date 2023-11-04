#pragma once

#include "painter.hpp"
#include "ui.hpp"

#include <SDL2/SDL.h>
#include <cstdio>
#include <memory>
#include <vector>

class EventHandler {
  public:
    EventHandler(Painter &painter, UserInferface &interface, std::vector<std::shared_ptr<GameObject>> &objects) : painter(painter), interface(interface), objects(objects){};

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
    Painter &painter;
    UserInferface &interface;
    std::vector<std::shared_ptr<GameObject>> &objects;
};
