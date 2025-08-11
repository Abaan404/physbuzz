#include "window.hpp"

#include "../debug/logging.hpp"
#include "../events/window.hpp"

namespace Physbuzz {

static void glfwErrorCallback(int error, const char *description) {
    Logger::ERROR("[GLFW] ({}) {}", error, description);
}

static inline std::unordered_map<GLFWwindow *, Window *> sWindowMap = {};

Window::Window(const Info &info)
    : m_Info(info) {}

Window::operator GLFWwindow *() const {
    return m_Window;
}

bool Window::operator==(const Window &other) const {
    return m_Window == other.m_Window;
}

bool Window::init() {
    // error callback
    glfwSetErrorCallback(glfwErrorCallback);

    // init glfw
    int isInit = glfwInit();
    if (isInit == GLFW_FALSE) {
        Logger::ERROR("[Window] Failed to initialize the GLFW library.");
        return false;
    }

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    return true;
}

bool Window::terminate() {
    glfwTerminate();
    return true;
}

bool Window::build(const vk::Instance &instance, const glm::ivec2 &resolution) {
    if (m_Window != nullptr) {
        Logger::ERROR("[Window] Could not create a constructed window.");
        return false;
    }

    m_Window = glfwCreateWindow(resolution.x, resolution.y, m_Info.title.c_str(), nullptr, nullptr);
    if (m_Window == nullptr) {
        Logger::ERROR("[Window] Could not create a GLFWwindow.");
        return false;
    }

    // static context for callbacks
    sWindowMap[m_Window] = this;

    glfwSetKeyCallback(m_Window, [](GLFWwindow *window, int key, int scancode, int action, int mods) {
        sWindowMap[window]->notifyCallbacks<KeyEvent>({.window = sWindowMap[window], .key = static_cast<Key>(key), .scancode = scancode, .action = static_cast<Action>(action), .mods = static_cast<Modifier>(mods)});
    });

    glfwSetCursorEnterCallback(m_Window, [](GLFWwindow *window, int entered) {
        sWindowMap[window]->notifyCallbacks<MouseEnteredEvent>({.window = sWindowMap[window], .entered = (entered == GLFW_TRUE)});
    });

    glfwSetScrollCallback(m_Window, [](GLFWwindow *window, double xoffset, double yoffset) {
        sWindowMap[window]->notifyCallbacks<MouseScrollEvent>({.window = sWindowMap[window], .offset = {xoffset, yoffset}});
    });

    glfwSetCursorPosCallback(m_Window, [](GLFWwindow *window, double xpos, double ypos) {
        sWindowMap[window]->notifyCallbacks<MousePositionEvent>({.window = sWindowMap[window], .position = {xpos, ypos}});
    });

    glfwSetMouseButtonCallback(m_Window, [](GLFWwindow *window, int button, int action, int mods) {
        sWindowMap[window]->notifyCallbacks<MouseButtonEvent>({.window = sWindowMap[window], .button = static_cast<Button>(button), .action = static_cast<Action>(action), .mods = static_cast<Modifier>(mods)});
    });

    glfwSetFramebufferSizeCallback(m_Window, [](GLFWwindow *window, int width, int height) {
        sWindowMap[window]->notifyCallbacks<WindowResizeEvent>({.window = sWindowMap[window], .resolution = {width, height}});
    });

    glfwSetWindowCloseCallback(m_Window, [](GLFWwindow *window) {
        sWindowMap[window]->notifyCallbacks<WindowCloseEvent>({.window = sWindowMap[window]});
        sWindowMap[window]->close(); // close the window when glfw requests it
    });

    glfwSetCharCallback(m_Window, [](GLFWwindow *window, unsigned int codepoint) {
        sWindowMap[window]->notifyCallbacks<CharEvent>({.window = sWindowMap[window], .codepoint = codepoint});
    });

    glfwSetWindowMaximizeCallback(m_Window, [](GLFWwindow *window, int maximized) {
        sWindowMap[window]->notifyCallbacks<WindowMaximizeEvent>({.window = sWindowMap[window], .maximized = static_cast<bool>(maximized & GLFW_MAXIMIZED)});
    });

    glfwSetDropCallback(m_Window, [](GLFWwindow *window, int path_count, const char *paths[]) {
        sWindowMap[window]->notifyCallbacks<MouseDropEvent>({.window = sWindowMap[window], .paths = {paths, paths + path_count}});
    });

    glfwSetWindowPosCallback(m_Window, [](GLFWwindow *window, int xpos, int ypos) {
        sWindowMap[window]->notifyCallbacks<WindowPositionEvent>({.window = sWindowMap[window], .position = {xpos, ypos}});
    });

    glfwSetWindowRefreshCallback(m_Window, [](GLFWwindow *window) {
        sWindowMap[window]->notifyCallbacks<WindowRefreshEvent>({.window = sWindowMap[window]});
    });

    glfwSetWindowFocusCallback(m_Window, [](GLFWwindow *window, int focused) {
        sWindowMap[window]->notifyCallbacks<WindowFocusEvent>({.window = sWindowMap[window], .focused = static_cast<bool>(focused & GLFW_FOCUSED)});
    });

    glfwSetWindowIconifyCallback(m_Window, [](GLFWwindow *window, int iconified) {
        sWindowMap[window]->notifyCallbacks<WindowIconifyEvent>({.window = sWindowMap[window], .iconified = static_cast<bool>(iconified & GLFW_ICONIFIED)});
    });

    // create a surface
    VkSurfaceKHR surface;
    glfwCreateWindowSurface(instance, m_Window, nullptr, &surface);
    m_Surface = surface;

    return true;
}

bool Window::destroy() {
    if (m_Window != nullptr) {
        Logger::ERROR("[Window] Could not destroy a destructed window.");
        return false;
    }

    close();
    glfwDestroyWindow(m_Window);
    sWindowMap.erase(m_Window);

    return true;
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

void Window::iconify() const {
    glfwIconifyWindow(m_Window);
}

void Window::restore() const {
    glfwRestoreWindow(m_Window);
}

void Window::maximize() const {
    glfwMaximizeWindow(m_Window);
}

void Window::poll() {
    glfwPollEvents();
}

void Window::setCursorPos(const glm::ivec2 &position) {
    glfwSetCursorPos(m_Window, position.x, position.y);
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

const Window::Info &Window::getInfo() const {
    return m_Info;
}

void Window::setCursorCapture(bool capture) const {
    if (capture) {
        glfwSetInputMode(m_Window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    } else {
        glfwSetInputMode(m_Window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    }
}

std::vector<const char *> Window::requiredExtensions() {
    // Get the required instance extensions from GLFW.
    std::uint32_t glfwExtensionCount = 0;
    const char **glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

    return {glfwExtensions, glfwExtensions + glfwExtensionCount};
}

} // namespace Physbuzz
