#include "window.hpp"

#include "debug/callbacks.hpp"
#include "logging.hpp"
#include <GLFW/glfw3.h>
#include <chrono>

namespace Physbuzz {

Window::Window() {
    // error callback
    glfwSetErrorCallback(glfwErrorCallback);

    // init glfw
    int isInit = glfwInit();
    Logger::ASSERT(isInit == GLFW_TRUE, "[GLFW] Could not initialize GLFW.");

    // setup window clock (glfw's time does not have ms precision)
    m_Clock = std::chrono::system_clock::now();

    // create a window.
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    m_Window = glfwCreateWindow(1280, 270, "PhysBuzz Engine", nullptr, nullptr);
    Logger::ASSERT(m_Window != nullptr, "[GLFW] Could not create a window.");

    // Setup OpenGL
    glfwMakeContextCurrent(m_Window);
    int version = gladLoadGL(glfwGetProcAddress);

    // debug context setup
    glEnable(GL_DEBUG_OUTPUT);
    glDebugMessageCallback(OpenGLDebugCallback, 0);

    glfwSwapInterval(1);
}

Window::~Window() {
    glfwSetWindowShouldClose(m_Window, GLFW_TRUE);
    glfwDestroyWindow(m_Window);
    glfwTerminate();
}

GLFWwindow *Window::getWindow() const {
    return m_Window;
}

glm::ivec2 Window::getResolution() const {
    int width;
    int height;

    glfwGetWindowSize(m_Window, &width, &height);
    return glm::ivec2(width, height);
}

glm::dvec2 Window::getCursorPos() const {
    double xpos, ypos;

    glfwGetCursorPos(m_Window, &xpos, &ypos);
    return glm::dvec2(xpos, ypos);
};

std::time_t Window::getTime() const {
    std::chrono::time_point<std::chrono::system_clock> now = std::chrono::system_clock::now();

    return std::chrono::duration_cast<std::chrono::milliseconds>(now - m_Clock).count();
}

void Window::setResolution(glm::ivec2 resolution) {
    glfwSetWindowSize(m_Window, resolution.x, resolution.y);
}

void Window::flip() {
    glfwSwapBuffers(m_Window);
}

} // namespace Physbuzz
