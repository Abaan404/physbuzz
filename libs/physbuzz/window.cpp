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
}

void Window::destroy() {
    close();
    glfwDestroyWindow(m_Window);
    glfwTerminate();
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

WindowEvents::WindowEvents(Window &window)
    : m_Window(window) {}

WindowEvents::WindowEvents(const WindowEvents &other)
    : m_Window(other.m_Window) {}

WindowEvents &WindowEvents::operator=(const WindowEvents &other) {
    if (this != &other) {
        m_Window = other.m_Window;
    }

    return *this;
}

void WindowEvents::poll() {
    glfwPollEvents();
}

void WindowEvents::setCallbackKeyEvent(std::function<void(KeyEvent)> callback) const {
    static std::function<void(KeyEvent)> tempCallback = callback;

    glfwSetKeyCallback(m_Window.getGLFWwindow(), [](GLFWwindow *window, int key, int scancode, int action, int mods) {
        std::function<void(KeyEvent)> callback = tempCallback;

        callback({
            .window = window,
            .key = key,
            .scancode = scancode,
            .action = action,
            .mods = mods,
        });
    });
}

void WindowEvents::setCallbackMouseEnteredEvent(std::function<void(MouseEnteredEvent)> callback) const {
    static std::function<void(MouseEnteredEvent)> tempCallback = callback;

    glfwSetCursorEnterCallback(m_Window.getGLFWwindow(), [](GLFWwindow *window, int entered) {
        std::function<void(MouseEnteredEvent)> callback = tempCallback;
        callback({
            .window = window,
            .entered = entered,
        });
    });
}

void WindowEvents::setCallbackMouseScrollEvent(std::function<void(MouseScrollEvent)> callback) const {
    static std::function<void(MouseScrollEvent)> tempCallback = callback;

    glfwSetScrollCallback(m_Window.getGLFWwindow(), [](GLFWwindow *window, double xoffset, double yoffset) {
        std::function<void(MouseScrollEvent)> callback = tempCallback;

        callback({
            .window = window,
            .xoffset = xoffset,
            .yoffset = yoffset,
        });
    });
}

void WindowEvents::setCallbackMousePositionEvent(std::function<void(MousePositionEvent)> callback) const {
    static std::function<void(MousePositionEvent)> tempCallback = callback;

    glfwSetCursorPosCallback(m_Window.getGLFWwindow(), [](GLFWwindow *window, double xpos, double ypos) {
        std::function<void(MousePositionEvent)> callback = tempCallback;

        callback({
            .window = window,
            .xpos = xpos,
            .ypos = ypos,
        });
    });
}

void WindowEvents::setCallbackMouseButtonEvent(std::function<void(MouseButtonEvent)> callback) const {
    static std::function<void(MouseButtonEvent)> tempCallback = callback;

    glfwSetMouseButtonCallback(m_Window.getGLFWwindow(), [](GLFWwindow *window, int button, int action, int mods) {
        std::function<void(MouseButtonEvent)> callback = tempCallback;

        callback({
            .window = window,
            .button = button,
            .action = action,
            .mods = mods,
        });
    });
}

void WindowEvents::setCallbackWindowResizeEvent(std::function<void(WindowResizeEvent)> callback) const {
    static std::function<void(WindowResizeEvent)> tempCallback = callback;

    glfwSetFramebufferSizeCallback(m_Window.getGLFWwindow(), [](GLFWwindow *window, int width, int height) {
        std::function<void(WindowResizeEvent)> callback = tempCallback;

        callback({
            .window = window,
            .width = width,
            .height = height,
        });
    });
}

void WindowEvents::setCallbackWindowCloseEvent(std::function<void(WindowCloseEvent)> callback) const {
    static std::function<void(WindowCloseEvent)> tempCallback = callback;

    glfwSetWindowCloseCallback(m_Window.getGLFWwindow(), [](GLFWwindow *window) {
        std::function<void(WindowCloseEvent)> callback = tempCallback;

        callback({
            .window = window,
        });
    });
}

void WindowEvents::setCallbackCharEvent(std::function<void(CharEvent)> callback) const {
    static std::function<void(CharEvent)> tempCallback = callback;

    glfwSetCharCallback(m_Window.getGLFWwindow(), [](GLFWwindow *window, unsigned int codepoint) {
        std::function<void(CharEvent)> callback = tempCallback;

        callback({
            .window = window,
            .codepoint = codepoint,
        });
    });
}

void WindowEvents::setCallbackMouseDropEvent(std::function<void(MouseDropEvent)> callback) const {
    static std::function<void(MouseDropEvent)> tempCallback = callback;

    glfwSetDropCallback(m_Window.getGLFWwindow(), [](GLFWwindow *window, int path_count, const char *paths[]) {
        std::function<void(MouseDropEvent)> callback = tempCallback;

        callback({
            .window = window,
            .paths = std::vector<std::string>(paths, paths + path_count),
        });
    });
}

void WindowEvents::setCallbackWindowPositionEvent(std::function<void(WindowPositionEvent)> callback) const {
    static std::function<void(WindowPositionEvent)> tempCallback = callback;

    glfwSetWindowPosCallback(m_Window.getGLFWwindow(), [](GLFWwindow *window, int xpos, int ypos) {
        std::function<void(WindowPositionEvent)> callback = tempCallback;

        callback({
            .window = window,
            .xpos = xpos,
            .ypos = ypos,
        });
    });
}

void WindowEvents::setCallbackWindowRefreshEvent(std::function<void(WindowRefreshEvent)> callback) const {
    static std::function<void(WindowRefreshEvent)> tempCallback = callback;

    glfwSetWindowRefreshCallback(m_Window.getGLFWwindow(), [](GLFWwindow *window) {
        std::function<void(WindowRefreshEvent)> callback = tempCallback;

        callback({
            .window = window,
        });
    });
}

void WindowEvents::setCallbackWindowFocusEvent(std::function<void(WindowFocusEvent)> callback) const {
    static std::function<void(WindowFocusEvent)> tempCallback = callback;

    glfwSetWindowFocusCallback(m_Window.getGLFWwindow(), [](GLFWwindow *window, int focused) {
        std::function<void(WindowFocusEvent)> callback = tempCallback;

        callback({
            .window = window,
            .focused = static_cast<bool>(focused & GLFW_FOCUSED),
        });
    });
}

void WindowEvents::setCallbackWindowIconifyEvent(std::function<void(WindowIconifyEvent)> callback) const {
    static std::function<void(WindowIconifyEvent)> tempCallback = callback;

    glfwSetWindowIconifyCallback(m_Window.getGLFWwindow(), [](GLFWwindow *window, int iconified) {
        std::function<void(WindowIconifyEvent)> callback = tempCallback;

        callback({
            .window = window,
            .iconified = static_cast<bool>(iconified & GLFW_ICONIFIED),
        });
    });
}

void WindowEvents::setCallbackWindowMaximizeEvent(std::function<void(WindowMaximizeEvent)> callback) const {
    static std::function<void(WindowMaximizeEvent)> tempCallback = callback;

    glfwSetWindowMaximizeCallback(m_Window.getGLFWwindow(), [](GLFWwindow *window, int maximized) {
        std::function<void(WindowMaximizeEvent)> callback = tempCallback;

        callback({
            .window = window,
            .maximized = static_cast<bool>(maximized & GLFW_MAXIMIZED),
        });
    });
}

} // namespace Physbuzz
