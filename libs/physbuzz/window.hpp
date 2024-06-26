#pragma once

#include <GLFW/glfw3.h>
#include <ctime>
#include <glm/glm.hpp>

namespace Physbuzz {

class Window {
  public:
    Window();
    ~Window();

    void build();
    void destroy();

    void close() const;
    void flip() const;
    bool shouldClose() const;

    GLFWwindow *getGLFWwindow() const;
    const glm::ivec2 getResolution() const;
    const glm::dvec2 getCursorPos() const;
    const std::time_t getTime() const;

    void setResolution(glm::ivec2 resolution);

  private:
    GLFWwindow *m_Window;

    friend class Renderer;
};

} // namespace Physbuzz
