#pragma once

#include "painter.hpp"
#include "ui.hpp"

#include <SDL2/SDL_events.h>
#include <functional>
#include <map>
#include <string>
#include <vector>

class EventHandler {
  public:
    EventHandler(Painter *painter, UserInferface *interface, std::vector<GameObject *> *objects) : painter(painter), interface(interface), objects(objects){};

    // execute all event in queue
    void execute();

    // sdl2 events
    void keyboard_keyup(SDL_KeyboardEvent &e);
    void keyboard_keydown(SDL_KeyboardEvent &e);
    void mouse_mousedown(SDL_MouseButtonEvent &e);
    void mouse_mouseup(SDL_MouseButtonEvent &e);
    void mouse_mousescroll(SDL_MouseButtonEvent &e);
    void mouse_mousemotion(SDL_MouseButtonEvent &e);
    void video_resize(SDL_Event &e);

  private:
    Painter *painter;
    UserInferface *interface;
    std::vector<GameObject *> *objects;
};
