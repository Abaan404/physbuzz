#pragma once

#include <GLFW/glfw3.h>

namespace Physbuzz {

// This is potentially unsafe and will let the user blast their leg off if used wrong, perhaps implement a way to allow only a unique T for this context
class Context {
  public:
    template <typename T>
    static void set(GLFWwindow *window, T *context) {
        glfwSetWindowUserPointer(window, context);
    }

    template <typename T>
    static T *get(GLFWwindow *window) {
        return (T *)glfwGetWindowUserPointer(window);
    }

  private:
    Context() = delete;
    Context(const Context &) = delete;
    Context(const Context &&) = delete;
    ~Context() = delete;
};

} // namespace Physbuzz
