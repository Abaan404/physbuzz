#pragma once

#include "../graphics/layout.hpp"
#include "../window/window.hpp"
#include <slang-com-ptr.h>
#include <vk_mem_alloc.h>
#include <vulkan/vulkan.hpp>

namespace Physbuzz {

class Scene;
class DeletionQueue;

class App {
  public:
    static bool init(const DescriptorLayoutAllocator::Info &layoutAllocatorInfo = {});
    static bool quit();

    // windowing
    static std::shared_ptr<Window> createWindow(const std::string &name, const Window::Info &windowInfo, const glm::ivec2 &resolution);
    static bool destroyWindow(const std::string &name);
    static std::shared_ptr<Window> getWindow(const std::string &name);

    // global ECS registry
    static Scene GScene;

    // global deletion queue
    static DeletionQueue Deletion;

    // global layout allocator
    static DescriptorLayoutAllocator LayoutAllocator;

  private:
    // Vulkan instances and extensions
    inline static vk::Instance Instance = nullptr;
    inline static vk::DebugUtilsMessengerEXT DebugMessenger = nullptr;

    // Device info
    inline static vk::PhysicalDevice PhysicalDevice = nullptr;
    inline static vk::Device Device = nullptr;
    inline static vk::PhysicalDeviceProperties PhysicalDeviceProperties = {};

    // VMA
    inline static VmaAllocator Allocator = VK_NULL_HANDLE;

    // slang
    inline static Slang::ComPtr<slang::IGlobalSession> SlangSession = nullptr;

    // tracy
    inline static TracyVkCtx Tracy = nullptr;

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

    friend class DeletionQueue;

    friend class DescriptorLayoutAllocator;
    friend class DescriptorLayout;

    template <PipelineType>
    friend class Pipeline;
    friend class GraphicsPipeline;
    friend class Shader;

    friend class Transfer;
    friend class Buffer;
    friend class Image;

    friend class StaticBuffer;
    friend class Texture;
    friend class Sampler;
    friend class Attachment;

    friend class Window;
    friend class Renderer;
    friend class ImGuiRenderer;
};

} // namespace Physbuzz
