#pragma once

#include "painter.hpp"
#include "store.hpp"
#include <SDL2/SDL_events.h>
#include <functional>
#include <map>
#include <string>

class EventHandler {
  public:
    EventHandler(Painter *painter, GameObjectStore *storage);

    // execute all event in queue
    void execute();

    // Adding/Removing events
    std::map<std::string, std::function<void()>> events;

    void register_event(std::string identifier, std::function<void()> callable);
    void toggle_event(std::string identifier, std::function<void()> callable);
    void release_event(std::string *identifier);

    // sdl2 events
    void keyboard_keyup(SDL_Event &e);
    void keyboard_keydown(SDL_Event &e);
    void mouse_mousedown(SDL_Event &e);
    void mouse_mouseup(SDL_Event &e);
    void mouse_mousescroll(SDL_Event &e);
    void mouse_mousemotion(SDL_Event &e);
    void video_resize(SDL_Event &e);

  private:
    Painter *painter;
    GameObjectStore *storage;
};
