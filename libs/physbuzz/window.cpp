#include "window.hpp"

#include "debug/callbacks.hpp"
#include "logging.hpp"
#include <GLFW/glfw3.h>
#include <string>

namespace Physbuzz {

Window::Window() {}

Window::~Window() {}

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

    glfwSetKeyCallback(m_Window, [](GLFWwindow *window, int key, int scancode, int action, int mods) {
        getCallback<KeyEvent>(window)({.window = window, .key = key, .scancode = scancode, .action = action, .mods = mods});
    });

    glfwSetCursorEnterCallback(m_Window, [](GLFWwindow *window, int entered) {
        getCallback<MouseEnteredEvent>(window)({.window = window, .entered = entered});
    });

    glfwSetScrollCallback(m_Window, [](GLFWwindow *window, double xoffset, double yoffset) {
        getCallback<MouseScrollEvent>(window)({.window = window, .xoffset = xoffset, .yoffset = yoffset});
    });

    glfwSetCursorPosCallback(m_Window, [](GLFWwindow *window, double xpos, double ypos) {
        getCallback<MousePositionEvent>(window)({.window = window, .xpos = xpos, .ypos = ypos});
    });

    glfwSetMouseButtonCallback(m_Window, [](GLFWwindow *window, int button, int action, int mods) {
        getCallback<MouseButtonEvent>(window)({.window = window, .button = button, .action = action, .mods = mods});
    });

    glfwSetFramebufferSizeCallback(m_Window, [](GLFWwindow *window, int width, int height) {
        getCallback<WindowResizeEvent>(window)({.window = window, .width = width, .height = height});
    });

    glfwSetWindowCloseCallback(m_Window, [](GLFWwindow *window) {
        getCallback<WindowCloseEvent>(window)({.window = window});
    });

    glfwSetCharCallback(m_Window, [](GLFWwindow *window, unsigned int codepoint) {
        getCallback<CharEvent>(window)({.window = window, .codepoint = codepoint});
    });

    glfwSetWindowMaximizeCallback(m_Window, [](GLFWwindow *window, int maximized) {
        getCallback<WindowMaximizeEvent>(window)({.window = window, .maximized = static_cast<bool>(maximized & GLFW_MAXIMIZED)});
    });

    glfwSetDropCallback(m_Window, [](GLFWwindow *window, int path_count, const char *paths[]) {
        getCallback<MouseDropEvent>(window)({.window = window, .paths = std::vector<std::string>(paths, paths + path_count)});
    });

    glfwSetWindowPosCallback(m_Window, [](GLFWwindow *window, int xpos, int ypos) {
        getCallback<WindowPositionEvent>(window)({.window = window, .xpos = xpos, .ypos = ypos});
    });

    glfwSetWindowRefreshCallback(m_Window, [](GLFWwindow *window) {
        getCallback<WindowRefreshEvent>(window)({.window = window});
    });

    glfwSetWindowFocusCallback(m_Window, [](GLFWwindow *window, int focused) {
        getCallback<WindowFocusEvent>(window)({.window = window, .focused = static_cast<bool>(focused & GLFW_FOCUSED)});
    });

    glfwSetWindowIconifyCallback(m_Window, [](GLFWwindow *window, int iconified) {
        getCallback<WindowIconifyEvent>(window)({.window = window, .iconified = static_cast<bool>(iconified & GLFW_ICONIFIED)});
    });
}

void Window::destroy() {
    close();
    glfwDestroyWindow(m_Window);
    glfwTerminate();

    getCallbacks<KeyEvent>().erase(m_Window);
    getCallbacks<WindowIconifyEvent>().erase(m_Window);
    getCallbacks<WindowFocusEvent>().erase(m_Window);
    getCallbacks<WindowRefreshEvent>().erase(m_Window);
    getCallbacks<WindowPositionEvent>().erase(m_Window);
    getCallbacks<MouseDropEvent>().erase(m_Window);
    getCallbacks<WindowMaximizeEvent>().erase(m_Window);
    getCallbacks<CharEvent>().erase(m_Window);
    getCallbacks<WindowCloseEvent>().erase(m_Window);
    getCallbacks<WindowResizeEvent>().erase(m_Window);
    getCallbacks<MouseButtonEvent>().erase(m_Window);
    getCallbacks<MousePositionEvent>().erase(m_Window);
    getCallbacks<MouseScrollEvent>().erase(m_Window);
    getCallbacks<MouseEnteredEvent>().erase(m_Window);
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

} // namespace Physbuzz
