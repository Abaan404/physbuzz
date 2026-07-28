#include "application.hpp"

#include "../ecs/scene.hpp"
#include "../graphics/descriptors/attachment.hpp"
#include "../graphics/descriptors/dynamic.hpp"
#include "../graphics/descriptors/static.hpp"
#include "../graphics/descriptors/texture.hpp"
#include "../graphics/layout.hpp"
#include "../graphics/material.hpp"
#include "../graphics/mesh.hpp"
#include "../graphics/pipeline.hpp"
#include "deletion.hpp"
#include <tracy/Tracy.hpp>

#define VMA_IMPLEMENTATION
#include <vk_mem_alloc.h>

VULKAN_HPP_DEFAULT_DISPATCH_LOADER_DYNAMIC_STORAGE

#ifdef PHYSBUZZ_ENABLE_TRACY

void *operator new(std ::size_t count) {
    auto ptr = malloc(count);
    TracyAlloc(ptr, count);
    return ptr;
}

void operator delete(void *ptr) noexcept {
    TracyFree(ptr);
    free(ptr);
}

#endif

namespace Physbuzz {

static VKAPI_ATTR vk::Bool32 VKAPI_CALL vulkanDebugCallback(vk::DebugUtilsMessageSeverityFlagBitsEXT severity, vk::DebugUtilsMessageTypeFlagsEXT type, const vk::DebugUtilsMessengerCallbackDataEXT *pCallbackData, void *) {
    if (pCallbackData->pMessage == nullptr) {
        return vk::False;
    }

    switch (severity) {
    case vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose:
    case vk::DebugUtilsMessageSeverityFlagBitsEXT::eInfo:
        Physbuzz::Logger::INFO("[Vulkan] ({}) {}", vk::to_string(type), pCallbackData->pMessage);
        break;

    case vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning:
        Physbuzz::Logger::WARNING("[Vulkan] ({}) {}", vk::to_string(type), pCallbackData->pMessage);
        break;

    case vk::DebugUtilsMessageSeverityFlagBitsEXT::eError:
        Physbuzz::Logger::ERROR("[Vulkan] ({}) {}", vk::to_string(type), pCallbackData->pMessage);
        break;
    }

    return vk::False;
}

DeletionQueue App::Deletion = {};
Scene App::GScene = {};
DescriptorLayoutAllocator App::LayoutAllocator = {{}};

bool App::init(const DescriptorLayoutAllocator::Info &layoutAllocatorInfo) {
    // setup logging
    Logger::init();

    // init slang
    SlangGlobalSessionDesc desc = {};
    if (SLANG_FAILED(slang::createGlobalSession(&desc, SlangSession.writeRef()))) {
        Logger::CRITICAL("[App] Failed to create a slang session.");
        return false;
    }

    // init glfw
    if (!Window::init()) {
        Logger::CRITICAL("[App] Failed to setup windowing.");
        return false;
    }

    // Get the required instance extensions from GLFW.
    std::vector<const char *> extensions = Window::requiredExtensions();
    std::vector<const char *> layers;

    VULKAN_HPP_DEFAULT_DISPATCHER.init();

#ifdef PHYSBUZZ_ENABLE_VALIDATION
    extensions.emplace_back(vk::EXTDebugUtilsExtensionName);
    layers.emplace_back("VK_LAYER_KHRONOS_validation");
#endif

    std::vector<vk::ExtensionProperties> extensionProperties = PBZ_VK_CHECK(vk::enumerateInstanceExtensionProperties());
    std::vector<vk::LayerProperties> layerProperties = PBZ_VK_CHECK(vk::enumerateInstanceLayerProperties());

    for (const auto &extension : extensions) {
        bool success = std::ranges::none_of(extensionProperties, [extension](const vk::ExtensionProperties &extensionProperty) {
            return strcmp(extensionProperty.extensionName, extension) == 0;
        });

        if (success) {
            Logger::CRITICAL("[App] Required extension not supported: {}", extension);
        }
    }

    for (const auto &layer : layers) {
        bool success = std::ranges::none_of(layerProperties, [layer](const vk::LayerProperties &layerProperty) {
            return strcmp(layerProperty.layerName, layer) == 0;
        });

        if (success) {
            Logger::CRITICAL("[App] Required layer not supported: {}", layer);
        }
    }

    // create the vulkan instance
    constexpr vk::ApplicationInfo appInfo = {
        .pNext = nullptr,
        .pApplicationName = "Game",
        .applicationVersion = vk::makeApiVersion(0, 0, 1, 0),
        .pEngineName = "Physbuzz",
        .engineVersion = vk::makeApiVersion(0, 0, 3, 0),
        .apiVersion = vk::ApiVersion14,
    };

    vk::InstanceCreateInfo createInfo = {
        .pApplicationInfo = &appInfo,
        .enabledLayerCount = static_cast<std::uint32_t>(layers.size()),
        .ppEnabledLayerNames = layers.data(),
        .enabledExtensionCount = static_cast<std::uint32_t>(extensions.size()),
        .ppEnabledExtensionNames = extensions.data(),
    };

    PBZ_VK_CHECK_RESULT(vk::createInstance(&createInfo, nullptr, &Instance));
    VULKAN_HPP_DEFAULT_DISPATCHER.init(Instance);

#ifdef PHYSBUZZ_ENABLE_VALIDATION
    vk::DebugUtilsMessengerCreateInfoEXT debugUtilsMessengerCreateInfoEXT = {
        .messageSeverity = vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose |
                           vk::DebugUtilsMessageSeverityFlagBitsEXT::eInfo |
                           vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning |
                           vk::DebugUtilsMessageSeverityFlagBitsEXT::eError,
        .messageType = vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral |
                       vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance |
                       vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation,
        .pfnUserCallback = &vulkanDebugCallback,
    };

    auto [debugMessengerResult, debugMessenger] = Instance.createDebugUtilsMessengerEXT(debugUtilsMessengerCreateInfoEXT);
    if (debugMessengerResult != vk::Result::eSuccess) {
        Logger::WARNING("[App] Failed to load debug validation layers.");
    } else {
        DebugMessenger = debugMessenger;
    }
#endif

    // get the device
    std::vector<vk::PhysicalDevice> devices = PBZ_VK_CHECK(Instance.enumeratePhysicalDevices());

    if (devices.empty()) {
        Logger::CRITICAL("[App] No GPU supporting Vulkan found.");
        return false;
    }

    // find an appropriate device
    std::multimap<int, vk::PhysicalDevice> deviceCandidates;
    for (const auto &device : devices) {
        auto deviceProperties = device.getProperties();
        std::uint32_t score = 0;

        // Discrete GPUs have a significant performance advantage
        if (deviceProperties.deviceType == vk::PhysicalDeviceType::eDiscreteGpu) {
            score += 1000;
        }

        if (deviceProperties.deviceType == vk::PhysicalDeviceType::eIntegratedGpu) {
            score += 100;
        }

        // Maximum possible size of textures affects graphics quality
        score += deviceProperties.limits.maxImageDimension2D;

        deviceCandidates.insert({score, device});
    }

    const auto [score, physicalDevice] = *deviceCandidates.rbegin();
    if (score <= 0) {
        Logger::CRITICAL("[App] failed to find a suitable GPU!.");
        return false;
    }

    PhysicalDevice = physicalDevice;
    PhysicalDeviceProperties = physicalDevice.getProperties();

    // find the needed queues
    std::vector<vk::QueueFamilyProperties> queueFamilyProperties = PhysicalDevice.getQueueFamilyProperties();

    for (std::size_t i = 0; i < queueFamilyProperties.size(); i++) {
        if ((queueFamilyProperties[i].queueFlags & (vk::QueueFlagBits::eGraphics | vk::QueueFlagBits::eCompute)) == (vk::QueueFlagBits::eGraphics | vk::QueueFlagBits::eCompute)) {
            // Assume graphics and present queues share the same index, this _could_ break for any physical
            // devices that would support present on other queues for some reason. We dont know the surface
            // the user will be using here yet. If a scenario like that happens, this needs will need a better
            // implementation.

            Indices.graphics = i;
            Indices.present = i;
        }

        else if (queueFamilyProperties[i].queueFlags & vk::QueueFlagBits::eTransfer) {
            Indices.transfer = i;
        }
    }

    if ((Indices.graphics == -1u) || (Indices.present == -1u)) {
        Logger::CRITICAL("[App] Could not find a queue family for graphics or present.");
        return false;
    }

    // if no dedicated transfer queue was found, use the graphics+compute family bit since they implicitly support transfer
    if (Indices.transfer == -1u) {
        Indices.transfer = Indices.graphics;
    }

    vk::PhysicalDeviceFeatures2 deviceFeatures = PhysicalDevice.getFeatures2();
    deviceFeatures.features.samplerAnisotropy = true;

    vk::StructureChain<
        vk::PhysicalDeviceFeatures2,
        vk::PhysicalDeviceVulkan11Features,
        vk::PhysicalDeviceVulkan12Features,
        vk::PhysicalDeviceVulkan13Features,
        vk::PhysicalDeviceVulkan14Features,
        vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT>
        deviceFeatureChain = {
            deviceFeatures,
            {
                .multiview = true,
                .shaderDrawParameters = true,
            },
            {
                .descriptorIndexing = true,
                .shaderSampledImageArrayNonUniformIndexing = true,
                .shaderStorageImageArrayNonUniformIndexing = true,
                .descriptorBindingSampledImageUpdateAfterBind = true,
                .descriptorBindingStorageImageUpdateAfterBind = true,
                .descriptorBindingPartiallyBound = true,
                .runtimeDescriptorArray = true,
                .scalarBlockLayout = true,
#ifdef PHYSBUZZ_ENABLE_TRACY
                .hostQueryReset = true,
#endif
                .bufferDeviceAddress = true,
            },
            {
                .synchronization2 = true,
                .dynamicRendering = true, // Enable dynamic rendering from Vulkan 1.3
            },
            {
                .dynamicRenderingLocalRead = true,
            },
            {
                .extendedDynamicState = true, // Enable extended dynamic state from the extension
            },
        };

    float queuePriority = 1.0f;
    std::vector<vk::DeviceQueueCreateInfo> queueCreateInfos;
    std::unordered_set<std::uint32_t> queueFamilyIndices = {Indices.graphics, Indices.present, Indices.transfer};

    for (const auto &queueFamilyIndex : queueFamilyIndices) {
        queueCreateInfos.push_back({
            .queueFamilyIndex = queueFamilyIndex,
            .queueCount = 1,
            .pQueuePriorities = &queuePriority,
        });
    }

    std::vector<const char *> deviceExtensions = {
        vk::KHRSwapchainExtensionName,
        vk::KHRSpirv14ExtensionName,
        vk::KHRSynchronization2ExtensionName,
        vk::KHRCreateRenderpass2ExtensionName,
    };

#ifdef PHYSBUZZ_ENABLE_TRACY
    deviceExtensions.emplace_back(vk::EXTCalibratedTimestampsExtensionName);
#endif

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmissing-designated-field-initializers" // ignore deprecated properties
    vk::DeviceCreateInfo deviceCreateInfo = {
        .pNext = &deviceFeatureChain.get(),
        .queueCreateInfoCount = static_cast<std::uint32_t>(queueCreateInfos.size()),
        .pQueueCreateInfos = queueCreateInfos.data(),
        .enabledExtensionCount = static_cast<std::uint32_t>(deviceExtensions.size()),
        .ppEnabledExtensionNames = deviceExtensions.data(),
    };
#pragma GCC diagnostic pop

    Device = PBZ_VK_CHECK(PhysicalDevice.createDevice(deviceCreateInfo));
    VULKAN_HPP_DEFAULT_DISPATCHER.init(Device);

    VmaVulkanFunctions vulkanFunctions{};
    vulkanFunctions.vkGetInstanceProcAddr = &vkGetInstanceProcAddr;
    vulkanFunctions.vkGetDeviceProcAddr = &vkGetDeviceProcAddr;

    VmaAllocatorCreateInfo allocatorInfo{};
    allocatorInfo.physicalDevice = PhysicalDevice;
    allocatorInfo.device = Device;
    allocatorInfo.instance = Instance;
    allocatorInfo.flags = VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT;
    allocatorInfo.pVulkanFunctions = &vulkanFunctions;

    vmaCreateAllocator(&allocatorInfo, &Allocator);

    Queues = {
        .graphics = Device.getQueue(Indices.graphics, 0),
        .present = Device.getQueue(Indices.present, 0),
        .transfer = Device.getQueue(Indices.transfer, 0),
    };

#ifdef PHYSBUZZ_ENABLE_TRACY
    PFN_vkGetPhysicalDeviceCalibrateableTimeDomainsEXT vkGetPhysicalDeviceCalibrateableTimeDomainsEXT =
        (PFN_vkGetPhysicalDeviceCalibrateableTimeDomainsEXT)
            vkGetInstanceProcAddr(App::Instance, "vkGetPhysicalDeviceCalibrateableTimeDomainsEXT");

    PFN_vkGetCalibratedTimestampsEXT vkGetCalibratedTimestampsEXT =
        (PFN_vkGetCalibratedTimestampsEXT)
            vkGetDeviceProcAddr(App::Device, "vkGetCalibratedTimestampsEXT");

    Tracy = TracyVkContextHostCalibrated(
        App::PhysicalDevice,
        App::Device,
        vkResetQueryPool,
        vkGetPhysicalDeviceCalibrateableTimeDomainsEXT,
        vkGetCalibratedTimestampsEXT);
#endif

    LayoutAllocator = layoutAllocatorInfo;

    if (!LayoutAllocator.build()) {
        Logger::CRITICAL("[App] Could not build pipeline layout allocator helper.");
    }

    return true;
}

bool App::quit() {
    PBZ_VK_CHECK_RESULT(App::Device.waitIdle());

    // free allocated pools and sets
    LayoutAllocator.destroy();

    // clear the scene completely
    GScene.clear();

    // destroy tracy context
#ifdef PHYSBUZZ_ENABLE_TRACY
    TracyVkDestroy(Tracy);
    Tracy = nullptr;
#endif

    // cleanup vulkan resources
    ResourceRegistry<GraphicsPipeline>::clear();
    ResourceRegistry<DescriptorLayout>::clear();
    ResourceRegistry<StaticBuffer>::clear();
    ResourceRegistry<DynamicBuffer>::clear();
    ResourceRegistry<Texture>::clear();
    ResourceRegistry<Attachment>::clear();
    ResourceRegistry<Sampler>::clear();
    ResourceRegistry<Mesh>::clear();
    ResourceRegistry<Material>::clear();

    vmaDestroyAllocator(Allocator);
    Deletion.flush();

    // destroy every window
    for (auto &[name, _] : m_Windows) {
        if (!getWindow(name)->destroy()) {
            Logger::ERROR("[App] Failed to destroy window.");
        }
    }

    m_Windows.clear();

    // quit glfw
    if (!Window::quit()) {
        Logger::ERROR("[App] Failed to destroy GLFW.");
        return false;
    }

#ifdef PHYSBUZZ_ENABLE_VALIDATION
    Instance.destroyDebugUtilsMessengerEXT(DebugMessenger);
    DebugMessenger = nullptr;
#endif

    // release vulkan resources
    Device.destroy();
    Device = nullptr;

    Instance.destroy();
    Instance = nullptr;

    return true;
}

std::shared_ptr<Window> App::createWindow(const std::string &name, const Window::Info &info, const glm::ivec2 &resolution) {
    m_Windows[name] = std::make_shared<Window>(info);

    if (!m_Windows[name]->build(resolution)) {
        Logger::CRITICAL("[App] Could not create a window.");
        return nullptr;
    }

    return m_Windows[name];
}

bool App::destroyWindow(const std::string &name) {
    if (!m_Windows.contains(name)) {
        return false;
    }

    m_Windows[name]->destroy();
    m_Windows.erase(name);

    return true;
}

std::shared_ptr<Window> App::getWindow(const std::string &name) {
    if (!m_Windows.contains(name)) {
        return nullptr;
    }

    return m_Windows[name];
}

} // namespace Physbuzz
