#pragma once

#include "events/window.hpp"
#include <GLFW/glfw3.h>
#include <functional>
#include <glm/glm.hpp>

namespace Physbuzz {

template <typename T>
concept IsWindowEventType =
    std::is_same_v<T, KeyEvent> ||
    std::is_same_v<T, WindowIconifyEvent> ||
    std::is_same_v<T, WindowFocusEvent> ||
    std::is_same_v<T, WindowRefreshEvent> ||
    std::is_same_v<T, WindowPositionEvent> ||
    std::is_same_v<T, MouseDropEvent> ||
    std::is_same_v<T, WindowMaximizeEvent> ||
    std::is_same_v<T, CharEvent> ||
    std::is_same_v<T, WindowCloseEvent> ||
    std::is_same_v<T, WindowResizeEvent> ||
    std::is_same_v<T, MouseButtonEvent> ||
    std::is_same_v<T, MousePositionEvent> ||
    std::is_same_v<T, MouseScrollEvent> ||
    std::is_same_v<T, MouseEnteredEvent>;

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

    void poll();

    template <typename T>
        requires IsWindowEventType<T>
    void setCallback(const std::function<void(const T &)> &callback) {
        std::unordered_map<GLFWwindow *, std::function<void(const T &)>> &callbacks = getCallbacks<T>();
        callbacks[m_Window] = callback;
    }

  private:
    template <typename T>
        requires IsWindowEventType<T>
    static std::unordered_map<GLFWwindow *, std::function<void(const T &)>> &getCallbacks() {
        static std::unordered_map<GLFWwindow *, std::function<void(const T &)>> callbacks;
        return callbacks;
    }

    template <typename T>
        requires IsWindowEventType<T>
    static std::function<void(const T &)> &getCallback(GLFWwindow *window) {
        std::unordered_map<GLFWwindow *, std::function<void(const T &)>> &callbacks = getCallbacks<T>();

        if (callbacks.contains(window)) {
            return callbacks[window];
        } else {
            static std::function<void(const T &)> emptyCallback = [](const T &) {};
            return emptyCallback;
        }
    }

    GLFWwindow *m_Window;
};

} // namespace Physbuzz
