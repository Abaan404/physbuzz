#pragma once

#include <physbuzz/events/window.hpp>

class Events {
  public:
    static void keyEvent(Physbuzz::KeyEvent event);
    static void mouseButton(Physbuzz::MouseButtonEvent event);
    static void WindowResize(Physbuzz::WindowResizeEvent event);
    static void WindowClose(Physbuzz::WindowCloseEvent event);
};
