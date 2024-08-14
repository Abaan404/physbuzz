#include "window.hpp"

#include "../debug/callbacks.hpp"
#include "../debug/logging.hpp"
#include "../events/window.hpp"
#include <GLFW/glfw3.h>
#include <string>

namespace Physbuzz {

Window::Window() {}

Window::~Window() {}

std::unordered_map<GLFWwindow *, Window *> Window::m_CallbackContexts = {};

void Window::build(const glm::ivec2 &resolution) {
    // error callback
    glfwSetErrorCallback(glfwErrorCallback);

    // init glfw
    int isInit = glfwInit();
    Logger::ASSERT(isInit == GLFW_TRUE, "[GLFW] Could not initialize GLFW.");

    // create a window.
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    m_Window = glfwCreateWindow(resolution.x, resolution.y, "PhysBuzz Engine", nullptr, nullptr);
    Logger::ASSERT(m_Window != nullptr, "[GLFW] Could not create a window.");

    // Setup OpenGL
    glfwMakeContextCurrent(m_Window);
    gladLoadGL(glfwGetProcAddress);

    // debug context setup
    glEnable(GL_DEBUG_OUTPUT);
    glDebugMessageCallback(OpenGLDebugCallback, 0);

    // static context for callbacks
    m_CallbackContexts[m_Window] = this;

    glfwSetKeyCallback(m_Window, [](GLFWwindow *window, int key, int scancode, int action, int mods) {
        m_CallbackContexts[window]->notifyCallbacks<KeyEvent>({.window = window, .key = static_cast<Key>(key), .scancode = scancode, .action = static_cast<Action>(action), .mods = static_cast<Modifier>(mods)});
    });

    glfwSetCursorEnterCallback(m_Window, [](GLFWwindow *window, int entered) {
        m_CallbackContexts[window]->notifyCallbacks<MouseEnteredEvent>({.window = window, .entered = (entered == GLFW_TRUE)});
    });

    glfwSetScrollCallback(m_Window, [](GLFWwindow *window, double xoffset, double yoffset) {
        m_CallbackContexts[window]->notifyCallbacks<MouseScrollEvent>({.window = window, .offset = {xoffset, yoffset}});
    });

    glfwSetCursorPosCallback(m_Window, [](GLFWwindow *window, double xpos, double ypos) {
        m_CallbackContexts[window]->notifyCallbacks<MousePositionEvent>({.window = window, .position = {xpos, ypos}});
    });

    glfwSetMouseButtonCallback(m_Window, [](GLFWwindow *window, int button, int action, int mods) {
        m_CallbackContexts[window]->notifyCallbacks<MouseButtonEvent>({.window = window, .button = static_cast<Mouse>(button), .action = static_cast<Action>(action), .mods = static_cast<Modifier>(mods)});
    });

    glfwSetFramebufferSizeCallback(m_Window, [](GLFWwindow *window, int width, int height) {
        m_CallbackContexts[window]->notifyCallbacks<WindowResizeEvent>({.window = window, .resolution = {width, height}});
    });

    glfwSetWindowCloseCallback(m_Window, [](GLFWwindow *window) {
        m_CallbackContexts[window]->notifyCallbacks<WindowCloseEvent>({.window = window});
    });

    glfwSetCharCallback(m_Window, [](GLFWwindow *window, unsigned int codepoint) {
        m_CallbackContexts[window]->notifyCallbacks<CharEvent>({.window = window, .codepoint = codepoint});
    });

    glfwSetWindowMaximizeCallback(m_Window, [](GLFWwindow *window, int maximized) {
        m_CallbackContexts[window]->notifyCallbacks<WindowMaximizeEvent>({.window = window, .maximized = static_cast<bool>(maximized & GLFW_MAXIMIZED)});
    });

    glfwSetDropCallback(m_Window, [](GLFWwindow *window, int path_count, const char *paths[]) {
        m_CallbackContexts[window]->notifyCallbacks<MouseDropEvent>({.window = window, .paths = std::vector<std::string>(paths, paths + path_count)});
    });

    glfwSetWindowPosCallback(m_Window, [](GLFWwindow *window, int xpos, int ypos) {
        m_CallbackContexts[window]->notifyCallbacks<WindowPositionEvent>({.window = window, .position = {xpos, ypos}});
    });

    glfwSetWindowRefreshCallback(m_Window, [](GLFWwindow *window) {
        m_CallbackContexts[window]->notifyCallbacks<WindowRefreshEvent>({.window = window});
    });

    glfwSetWindowFocusCallback(m_Window, [](GLFWwindow *window, int focused) {
        m_CallbackContexts[window]->notifyCallbacks<WindowFocusEvent>({.window = window, .focused = static_cast<bool>(focused & GLFW_FOCUSED)});
    });

    glfwSetWindowIconifyCallback(m_Window, [](GLFWwindow *window, int iconified) {
        m_CallbackContexts[window]->notifyCallbacks<WindowIconifyEvent>({.window = window, .iconified = static_cast<bool>(iconified & GLFW_ICONIFIED)});
    });
}

void Window::destroy() {
    close();
    glfwDestroyWindow(m_Window);
    glfwTerminate();

    m_CallbackContexts.erase(m_Window);
}

void Window::close() const {
    glfwSetWindowShouldClose(m_Window, GLFW_TRUE);
}

void Window::setTitle(const std::string &title) const {
    glfwSetWindowTitle(m_Window, title.c_str());
}

void Window::setPos(const glm::ivec2 &position) const {
    glfwSetWindowPos(m_Window, position.x, position.y);
}

void Window::swapInterval(int interval) const {
    glfwSwapInterval(interval);
}

void Window::iconify() const {
    glfwIconifyWindow(m_Window);
}

void Window::restore() const {
    glfwRestoreWindow(m_Window);
}

void Window::maximize() const {
    glfwMaximizeWindow(m_Window);
}

void Window::setCursorPos(const glm::ivec2 &position) {
    glfwSetCursorPos(m_Window, position.x, position.y);
}

GLFWwindow *Window::getGLFWwindow() const {
    return m_Window;
}

const glm::ivec2 Window::getResolution() const {
    int width;
    int height;

    glfwGetWindowSize(m_Window, &width, &height);
    return glm::ivec2(width, height);
}

const glm::dvec2 Window::getCursorPos() const {
    double xpos, ypos;

    glfwGetCursorPos(m_Window, &xpos, &ypos);
    return glm::dvec2(xpos, ypos);
};

bool Window::shouldClose() const {
    return glfwWindowShouldClose(m_Window);
}

void Window::setResolution(const glm::ivec2 &resolution) {
    glfwSetWindowSize(m_Window, resolution.x, resolution.y);
}

void Window::flip() const {
    glfwSwapBuffers(m_Window);
}

void Window::poll() {
    glfwPollEvents();
}

void Window::setCursorCapture(bool capture) const {
    if (capture) {
        glfwSetInputMode(m_Window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    } else {
        glfwSetInputMode(m_Window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    }
}

} // namespace Physbuzz
