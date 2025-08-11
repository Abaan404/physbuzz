#pragma once

#include "../events/handler.hpp"
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <vector>
#include <vulkan/vulkan.hpp>

namespace Physbuzz {

class Window : public EventSubject {
  public:
    using Format = vk::Format;
    using ColorSpace = vk::ColorSpaceKHR;
    using PresentMode = vk::PresentModeKHR;

    struct Info {
        std::string title = "Physbuzz Game";

        struct {
            Format format = vk::Format::eB8G8R8A8Srgb;
            ColorSpace colorSpace = ColorSpace::eSrgbNonlinear;
            PresentMode presentMode = PresentMode::eMailbox;
        } swapChain = {};
    };

    Window(const Info &info);

    operator GLFWwindow *() const;
    bool operator==(const Window &other) const;

    void setTitle(const std::string &title) const;
    void setPos(const glm::ivec2 &position) const;

    void iconify() const;
    void maximize() const;
    void restore() const;

    void close() const;
    void flip() const;
    bool shouldClose() const;
    void poll();

    void setCursorCapture(bool capture) const;

    const glm::dvec2 getCursorPos() const;
    void setCursorPos(const glm::ivec2 &position);

    const glm::ivec2 getResolution() const;
    void setResolution(const glm::ivec2 &resolution);

    const Info &getInfo() const;

  private:
    static bool init();
    static bool terminate();
    static std::vector<const char *> requiredExtensions();

    // these functions can only be called with App::createWindow()
    bool build(const vk::Instance &instance, const glm::ivec2 &resolution);
    bool destroy();

    GLFWwindow *m_Window = nullptr;
    vk::SurfaceKHR m_Surface = nullptr;

    // Vulkan objects (created by App)
    vk::SwapchainKHR m_SwapChain = nullptr;
    std::vector<vk::ImageView> m_SwapChainViews = {};

    Info m_Info;

    friend class App;
};

} // namespace Physbuzz
