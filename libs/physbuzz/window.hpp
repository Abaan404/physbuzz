#pragma once

#include "events/window.hpp"
#include <GLFW/glfw3.h>
#include <functional>
#include <glm/glm.hpp>

namespace Physbuzz {

class Window {
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

    GLFWwindow *getGLFWwindow() const;

    const glm::dvec2 getCursorPos() const;
    void setCursorPos(const glm::ivec2 &position);

    const glm::ivec2 getResolution() const;
    void setResolution(const glm::ivec2 &resolution);

  private:
    GLFWwindow *m_Window;
};

class WindowEvents {
  public:
    WindowEvents(Window &window);
    WindowEvents(const WindowEvents &other);
    WindowEvents &operator=(const WindowEvents &other);

    void poll();

    void setCallbackKeyEvent(std::function<void(KeyEvent)> callback) const;
    void setCallbackCharEvent(std::function<void(CharEvent)> callback) const;

    void setCallbackMouseButtonEvent(std::function<void(MouseButtonEvent)> callback) const;
    void setCallbackMousePositionEvent(std::function<void(MousePositionEvent)> callback) const;
    void setCallbackMouseEnteredEvent(std::function<void(MouseEnteredEvent)> callback) const;
    void setCallbackMouseScrollEvent(std::function<void(MouseScrollEvent)> callback) const;
    void setCallbackMouseDropEvent(std::function<void(MouseDropEvent)> callback) const;

    void setCallbackWindowPositionEvent(std::function<void(WindowPositionEvent)> callback) const;
    void setCallbackWindowResizeEvent(std::function<void(WindowResizeEvent)> callback) const;
    void setCallbackWindowCloseEvent(std::function<void(WindowCloseEvent)> callback) const;
    void setCallbackWindowRefreshEvent(std::function<void(WindowRefreshEvent)> callback) const;
    void setCallbackWindowFocusEvent(std::function<void(WindowFocusEvent)> callback) const;
    void setCallbackWindowIconifyEvent(std::function<void(WindowIconifyEvent)> callback) const;
    void setCallbackWindowMaximizeEvent(std::function<void(WindowMaximizeEvent)> callback) const;

  private:
    Window &m_Window;
};

} // namespace Physbuzz
