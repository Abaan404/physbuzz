#pragma once

#include <GLFW/glfw3.h>
#include <string>
#include <vector>

namespace Physbuzz {

struct KeyEvent {
    GLFWwindow *window;

    int key;
    int scancode;
    int action;
    int mods;
};

struct CharEvent {
    GLFWwindow *window;

    unsigned int codepoint;
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

struct MouseDropEvent {
    GLFWwindow *window;

    std::vector<std::string> paths;
    // int path_count;
    // const char *paths[];
};

struct WindowPositionEvent {
    GLFWwindow *window;

    int xpos;
    int ypos;
};

struct WindowResizeEvent {
    GLFWwindow *window;

    int width;
    int height;
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
    // int focused;
};

struct WindowIconifyEvent {
    GLFWwindow *window;

    bool iconified;
    // int iconified;
};

struct WindowMaximizeEvent {
    GLFWwindow *window;

    bool maximized;
    // int maximized;
};

}; // namespace Physbuzz
