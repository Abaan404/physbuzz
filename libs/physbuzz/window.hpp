#pragma once

#include "events.hpp"
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

namespace Physbuzz {

class Window : public IEventSubject {
  public:
    Window();
    ~Window();

    void build(const glm::ivec2 &resolution);
    void destroy();

    void setTitle(const std::string &title) const;
    void setPos(const glm::ivec2 &position) const;
    void swapInterval(int interval) const;

    void iconify() const;
    void maximize() const;
    void restore() const;

    void close() const;
    void flip() const;
    bool shouldClose() const;

    void setCursorCapture(bool capture) const;

    GLFWwindow *getGLFWwindow() const;

    const glm::dvec2 getCursorPos() const;
    void setCursorPos(const glm::ivec2 &position);

    const glm::ivec2 getResolution() const;
    void setResolution(const glm::ivec2 &resolution);

    void poll();

  private:
    static std::unordered_map<GLFWwindow *, Window *> m_CallbackContexts;
    GLFWwindow *m_Window;
};

} // namespace Physbuzz
