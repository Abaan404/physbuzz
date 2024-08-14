#pragma once

#include "../window/input.hpp"
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <string>
#include <vector>

namespace Physbuzz {

struct KeyEvent {
    GLFWwindow *window;

    Key key;
    int scancode;
    Action action;
    Modifier mods;
};

struct CharEvent {
    GLFWwindow *window;

    unsigned int codepoint;
};

struct MouseButtonEvent {
    GLFWwindow *window;

    Mouse button;
    Action action;
    Modifier mods;
};

struct MousePositionEvent {
    GLFWwindow *window;

    glm::dvec2 position;
};

struct MouseEnteredEvent {
    GLFWwindow *window;

    bool entered;
};

struct MouseScrollEvent {
    GLFWwindow *window;

    glm::dvec2 offset;
};

struct MouseDropEvent {
    GLFWwindow *window;

    std::vector<std::string> paths;
};

struct WindowPositionEvent {
    GLFWwindow *window;

    glm::ivec2 position;
};

struct WindowResizeEvent {
    GLFWwindow *window;

    glm::ivec2 resolution;
};

struct WindowCloseEvent {
    GLFWwindow *window;
};

struct WindowRefreshEvent {
    GLFWwindow *window;
};

struct WindowFocusEvent {
    GLFWwindow *window;

    bool focused;
};

struct WindowIconifyEvent {
    GLFWwindow *window;

    bool iconified;
};

struct WindowMaximizeEvent {
    GLFWwindow *window;

    bool maximized;
};

}; // namespace Physbuzz
