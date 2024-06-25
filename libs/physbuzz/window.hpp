#pragma once
#define GLFW_INCLUDE_NONE

#include <GLFW/glfw3.h>
#include <chrono>
#include <ctime>
#include <glm/glm.hpp>

namespace Physbuzz {

class Window {
  public:
    Window();
    ~Window();

    void build();
    void destroy();
    void flip();

    GLFWwindow *getWindow() const;
    glm::ivec2 getResolution() const;
    glm::dvec2 getCursorPos() const;

    void setResolution(glm::ivec2 resolution);
    std::time_t getTime() const;

  private:
    GLFWwindow *m_Window;

    std::chrono::time_point<std::chrono::system_clock> m_Clock;

    friend class Renderer;
    friend class EventManager;
};

} // namespace Physbuzz
