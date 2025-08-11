#pragma once

#include "../ecs/scene.hpp"
#include "../window/window.hpp"
#include <list>
#include <vulkan/vulkan.hpp>

namespace Physbuzz {

class App {
  public:
    static bool build();
    static bool destroy();

    static Scene &getGlobalScene();

    static std::shared_ptr<Window> createWindow(const Window::Info &windowInfo, const glm::ivec2 &resolution);
    static bool destroyWindow(const std::shared_ptr<Window> &window);
    static const std::list<std::shared_ptr<Window>> &getWindows();

  private:
    // ECS registry
    inline static Scene m_Scene;

    // Vulkan instances and extensions
    inline static vk::Instance m_Instance = nullptr;
    inline static vk::DebugUtilsMessengerEXT m_DebugMessanger = nullptr;

    // Device info
    inline static vk::PhysicalDevice m_PhysicalDevice = nullptr;
    inline static vk::Device m_Device = nullptr;

    inline static struct {
        std::uint32_t graphics;
        std::uint32_t present;
    } m_Indices = {
        .graphics = 0,
        .present = 0,
    };

    inline static vk::Queue m_GraphicsQueue = nullptr;
    inline static vk::Queue m_PresentQueue = nullptr;

    // inline static struct {
    //     vk::Queue queue;
    //     std::uint32_t familyIndex;
    //     float priority;
    // } m_GraphicsQueue = {};

    // windows
    inline static std::list<std::shared_ptr<Window>> m_Windows;
};

} // namespace Physbuzz
