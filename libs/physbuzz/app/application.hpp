#pragma once

#include "../ecs/scene.hpp"
#include "../window/window.hpp"
#include <vulkan/vulkan.hpp>

namespace Physbuzz {

class App {
  public:
    static bool init();
    static bool quit();

    // windowing
    static std::shared_ptr<Window> createWindow(const std::string &name, const Window::Info &windowInfo, const glm::ivec2 &resolution);
    static bool destroyWindow(const std::string &name);
    static std::shared_ptr<Window> getWindow(const std::string &name);

    // global ECS registry
    inline static Scene GScene;

  private:
    // Vulkan instances and extensions
    inline static vk::Instance Instance = nullptr;
    inline static vk::DebugUtilsMessengerEXT DebugMessenger = nullptr;

    // Device info
    inline static vk::PhysicalDevice PhysicalDevice = nullptr;
    inline static vk::Device Device = nullptr;

    inline static struct {
        std::uint32_t graphics;
        std::uint32_t present;
        std::uint32_t transfer;
    } Indices = {
        .graphics = -1u,
        .present = -1u,
        .transfer = -1u,
    };

    inline static struct {
        vk::Queue graphics;
        vk::Queue present;
        vk::Queue transfer;
    } Queues = {
        .graphics = nullptr,
        .present = nullptr,
        .transfer = nullptr,
    };

    // windows
    inline static std::unordered_map<std::string, std::shared_ptr<Window>> m_Windows;

    friend class Transfer;
    friend class Buffer;

    friend class Window;
    friend class ShaderPipeline;
    friend class Renderer;
};

} // namespace Physbuzz
