#include "events.hpp"

namespace Physbuzz {

EventManager::EventManager(Window &window)
    : m_Window(window) {}

EventManager::EventManager(const EventManager &other)
    : m_Window(other.m_Window) {}

EventManager &EventManager::operator=(const EventManager &other) {
    if (this != &other) {
        m_Window = other.m_Window;
    }

    return *this;
}

void EventManager::poll() {
    glfwPollEvents();
}

void EventManager::setCallbackKeyEvent(std::function<void(KeyEvent)> callback) const {
    CallbackKeyEvent = callback;

    glfwSetKeyCallback(m_Window.getGLFWwindow(), [](GLFWwindow *window, int key, int scancode, int action, int mods) {
        CallbackKeyEvent({
            .window = window,
            .key = key,
            .scancode = scancode,
            .action = action,
            .mods = mods,
        });
    });
};

void EventManager::setCallbackMouseEnteredEvent(std::function<void(MouseEnteredEvent)> callback) const {
    CallbackMouseEnteredEvent = callback;

    glfwSetCursorEnterCallback(m_Window.getGLFWwindow(), [](GLFWwindow *window, int entered) {
        CallbackMouseEnteredEvent({
            .window = window,
            .entered = entered,
        });
    });
};

void EventManager::setCallbackMouseScrollEvent(std::function<void(MouseScrollEvent)> callback) const {
    CallbackMouseScrollEvent = callback;

    glfwSetScrollCallback(m_Window.getGLFWwindow(), [](GLFWwindow *window, double xoffset, double yoffset) {
        CallbackMouseScrollEvent({
            .window = window,
            .xoffset = xoffset,
            .yoffset = yoffset,
        });
    });
};

void EventManager::setCallbackMouseMotionEvent(std::function<void(MousePositionEvent)> callback) const {
    CallbackMousePositionEvent = callback;

    glfwSetCursorPosCallback(m_Window.getGLFWwindow(), [](GLFWwindow *window, double xpos, double ypos) {
        CallbackMousePositionEvent({
            .window = window,
            .xpos = xpos,
            .ypos = ypos,
        });
    });
};

void EventManager::setCallbackMouseButtonEvent(std::function<void(MouseButtonEvent)> callback) const {
    CallbackMouseButtonEvent = callback;

    glfwSetMouseButtonCallback(m_Window.getGLFWwindow(), [](GLFWwindow *window, int button, int action, int mods) {
        CallbackMouseButtonEvent({
            .window = window,
            .button = button,
            .action = action,
            .mods = mods,
        });
    });
};

void EventManager::setCallbackWindowResize(std::function<void(WindowResizeEvent)> callback) const {
    CallbackWindowResizeEvent = callback;

    glfwSetFramebufferSizeCallback(m_Window.getGLFWwindow(), [](GLFWwindow *window, int width, int height) {
        CallbackWindowResizeEvent({
            .window = window,
            .width = width,
            .height = height,
        });
    });
}

void EventManager::setCallbackWindowClose(std::function<void(WindowCloseEvent)> callback) const {
    CallbackWindowCloseEvent = callback;

    glfwSetWindowCloseCallback(m_Window.getGLFWwindow(), [](GLFWwindow *window) {
        CallbackWindowResizeEvent({
            .window = window,
        });
    });
};

} // namespace Physbuzz
