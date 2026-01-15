#pragma once

#include "../window/defines.hpp"
#include <filesystem>
#include <glm/glm.hpp>
#include <vector>

namespace Physbuzz {

class Window;

struct KeyEvent {
    Window *window;

    Key key;
    int scancode;
    Action action;
    Modifier mods;
};

struct CharEvent {
    Window *window;

    unsigned int codepoint;
};

struct MouseButtonEvent {
    Window *window;

    Button button;
    Action action;
    Modifier mods;
};

struct MousePositionEvent {
    Window *window;

    glm::dvec2 position;
};

struct MouseEnteredEvent {
    Window *window;

    bool entered;
};

struct MouseScrollEvent {
    Window *window;

    glm::dvec2 offset;
};

struct MouseDropEvent {
    Window *window;

    std::vector<std::filesystem::path> paths;
};

struct WindowPositionEvent {
    Window *window;

    glm::ivec2 position;
};

struct WindowResizeEvent {
    Window *window;

    glm::ivec2 resolution;
};

struct WindowSwapchainResizeEvent {
    Window *window;

    glm::ivec2 resolution;
};

struct WindowCloseEvent {
    Window *window;
};

struct WindowRefreshEvent {
    Window *window;
};

struct WindowFocusEvent {
    Window *window;

    bool focused;
};

struct WindowIconifyEvent {
    Window *window;

    bool iconified;
};

struct WindowMaximizeEvent {
    Window *window;

    bool maximized;
};

}; // namespace Physbuzz
