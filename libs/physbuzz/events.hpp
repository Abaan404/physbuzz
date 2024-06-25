#pragma once

#include "window.hpp"
#include <functional>

// TODO:
//  - Map all glfw events
//  - Template to make this managable?

namespace Physbuzz {

struct KeyEvent {
    GLFWwindow *window;

    int key;
    int scancode;
    int action;
    int mods;
};

struct MouseButtonEvent {
    GLFWwindow *window;

    int button;
    int action;
    int mods;
};

struct MousePositionEvent {
    GLFWwindow *window;

    double xpos;
    double ypos;
};

struct MouseEnteredEvent {
    GLFWwindow *window;

    int entered;
};

struct MouseScrollEvent {
    GLFWwindow *window;

    double xoffset;
    double yoffset;
};

struct WindowResizeEvent {
    GLFWwindow *window;

    int width;
    int height;
};

struct WindowCloseEvent {
    GLFWwindow *window;
};

class EventManager {
  public:
    EventManager(Window &window);
    EventManager(const EventManager &other);
    EventManager &operator=(const EventManager &other);

    void poll();

    void setCallbackKeyEvent(std::function<void(KeyEvent)> callback) const;

    void setCallbackMouseButtonEvent(std::function<void(MouseButtonEvent)> callback) const;
    void setCallbackMouseMotionEvent(std::function<void(MousePositionEvent)> callback) const;
    void setCallbackMouseScrollEvent(std::function<void(MouseScrollEvent)> callback) const;
    void setCallbackMouseEnteredEvent(std::function<void(MouseEnteredEvent)> callback) const;

    void setCallbackWindowResize(std::function<void(WindowResizeEvent)> callback) const;
    void setCallbackWindowClose(std::function<void(WindowCloseEvent)> callback) const;

  private:
    Window &m_Window;
};

namespace {

auto defaultCallback = [](auto param) {};

// these simply exist as temporary callback variables that exist in a defined
// memory to avoid using scoped lambdas and let glfw use the function pointer
static std::function<void(KeyEvent)> CallbackKeyEvent = defaultCallback;
static std::function<void(MouseEnteredEvent)> CallbackMouseEnteredEvent = defaultCallback;
static std::function<void(MouseScrollEvent)> CallbackMouseScrollEvent = defaultCallback;
static std::function<void(MousePositionEvent)> CallbackMousePositionEvent = defaultCallback;
static std::function<void(MouseButtonEvent)> CallbackMouseButtonEvent = defaultCallback;
static std::function<void(WindowResizeEvent)> CallbackWindowResizeEvent = defaultCallback;
static std::function<void(WindowCloseEvent)> CallbackWindowCloseEvent = defaultCallback;
} // namespace

} // namespace Physbuzz
