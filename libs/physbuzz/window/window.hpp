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
            Format format = vk::Format::eR8G8B8A8Srgb;
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
    void poll() const;

    void setCursorCapture(bool capture) const;

    const glm::dvec2 getCursorPos() const;
    void setCursorPos(const glm::ivec2 &position);

    const glm::uvec2 getResolution() const;
    void setResolution(const glm::uvec2 &resolution);

    const Info &getInfo() const;

  private:
    void buildSwapChain();
    void destroySwapChain();
    void recreateSwapChain();

    Info m_Info;
    GLFWwindow *m_Window = nullptr;

    vk::SurfaceKHR m_Surface = nullptr;
    vk::SwapchainKHR m_SwapChain = nullptr;
    std::vector<vk::Image> m_SwapChainImages = {};
    std::vector<vk::ImageView> m_SwapChainImageViews = {};

    bool m_FramebufferResized = false;
    glm::uvec2 m_SwapChainExtent = {0, 0};

    friend class App;
    friend class Renderer;

    // these functions can only be called by App()
    static bool init();
    static bool quit();

    bool build(const glm::ivec2 &resolution);
    bool destroy();

    static std::vector<const char *> requiredExtensions();
};

} // namespace Physbuzz
