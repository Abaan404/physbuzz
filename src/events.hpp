#pragma once

#include <physbuzz/events/window.hpp>

class Game;

class Events {
  public:
    Events(Game &game);
    ~Events();

    void build();
    void destroy();

    void keyEvent(const Physbuzz::KeyEvent &event);
    void mouseButton(const Physbuzz::MouseButtonEvent &event);
    void WindowResize(const Physbuzz::WindowResizeEvent &event);
    void WindowClose(const Physbuzz::WindowCloseEvent &event);

  private:
    Game &m_Game;
};
