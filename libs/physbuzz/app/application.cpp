#include "application.hpp"

#include "../debug/macros.hpp"
#include <algorithm>
#include <array>
#include <glm/common.hpp>
#include <map>
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_enums.hpp>

VULKAN_HPP_DEFAULT_DISPATCH_LOADER_DYNAMIC_STORAGE

#if !defined(NDEBUG)
#define ENABLE_VALIDATION_LAYERS
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

bool App::init() {
    // setup logging
    Logger::init();

    // init glfw
    if (!Window::init()) {
        Logger::CRITICAL("[App] Failed to setup windowing.");
        return false;
    }

    // Get the required instance extensions from GLFW.
    std::vector<const char *> extensions = Window::requiredExtensions();

    VULKAN_HPP_DEFAULT_DISPATCHER.init();

#ifdef ENABLE_VALIDATION_LAYERS
    extensions.emplace_back(vk::EXTDebugUtilsExtensionName);
#endif

    std::vector<vk::ExtensionProperties> extensionProperties = PBZ_VK_CHECK(vk::enumerateInstanceExtensionProperties());

    for (const auto &extension : extensions) {
        bool success = std::ranges::none_of(extensionProperties, [extension](const VkExtensionProperties &extensionProperty) {
            return strcmp(extensionProperty.extensionName, extension) == 0;
        });

        if (success) {
            Logger::CRITICAL("[App] Required GLFW extensions not supported: {}", std::string_view(extension));
            return false;
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
        .enabledExtensionCount = static_cast<std::uint32_t>(extensions.size()),
        .ppEnabledExtensionNames = extensions.data(),
    };

    if (vk::createInstance(&createInfo, nullptr, &Instance) != vk::Result::eSuccess) {
        Logger::ERROR("[App] Failed to create vulkan instance.");
        return false;
    }

    VULKAN_HPP_DEFAULT_DISPATCHER.init(Instance);

#ifdef ENABLE_VALIDATION_LAYERS
    vk::DebugUtilsMessengerCreateInfoEXT debugUtilsMessengerCreateInfoEXT{
        .messageSeverity = vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose |
                           vk::DebugUtilsMessageSeverityFlagBitsEXT::eInfo |
                           vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning |
                           vk::DebugUtilsMessageSeverityFlagBitsEXT::eError,
        .messageType = vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral |
                       vk::DebugUtilsMessageTypeFlagBitsEXT::eDeviceAddressBinding |
                       vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance |
                       vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation,
        .pfnUserCallback = &vulkanDebugCallback};

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

    // find the needed queues
    std::vector<vk::QueueFamilyProperties> queueFamilyProperties = PhysicalDevice.getQueueFamilyProperties();

    m_Indices.graphics = 0;
    for (const auto &props : queueFamilyProperties) {
        if (props.queueFlags & vk::QueueFlagBits::eGraphics) {
            break;
        }

        m_Indices.graphics++;
    }

    // Assume graphics and present queues share the same index, this _could_ break for any physical devices that would support present on other queues for some reason.
    // We dont know the surface the user will be using here yet. If a scenario like that happens, this needs will need a better implementation.
    m_Indices.present = m_Indices.graphics;

    if ((m_Indices.graphics == queueFamilyProperties.size()) || (m_Indices.present == queueFamilyProperties.size())) {
        Logger::CRITICAL("[App] Could not find a queue for graphics or present.");
        return false;
    }

    vk::StructureChain<vk::PhysicalDeviceFeatures2, vk::PhysicalDeviceVulkan13Features, vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT> deviceFeatureChain = {
        {
            PhysicalDevice.getFeatures2(),
        },
        {
            .dynamicRendering = true, // Enable dynamic rendering from Vulkan 1.3
        },
        {
            .extendedDynamicState = true, // Enable extended dynamic state from the extension
        },
    };

    constexpr std::array graphicsQueuePriorities = {0.0f};

    std::vector<vk::DeviceQueueCreateInfo> queueCreateInfos = {
        {{
            .queueFamilyIndex = m_Indices.graphics,
            .queueCount = graphicsQueuePriorities.size(),
            .pQueuePriorities = graphicsQueuePriorities.data(),
        }},
    };

    std::vector<const char *> deviceExtensions = {
        vk::KHRSwapchainExtensionName,
        vk::KHRSpirv14ExtensionName,
        vk::KHRSynchronization2ExtensionName,
        vk::KHRCreateRenderpass2ExtensionName,
    };

    vk::DeviceCreateInfo deviceCreateInfo = {
        .pNext = &deviceFeatureChain.get<vk::PhysicalDeviceFeatures2>(),
        .queueCreateInfoCount = static_cast<std::uint32_t>(queueCreateInfos.size()),
        .pQueueCreateInfos = queueCreateInfos.data(),
        .enabledExtensionCount = static_cast<std::uint32_t>(deviceExtensions.size()),
        .ppEnabledExtensionNames = deviceExtensions.data(),
    };

    Device = PBZ_VK_CHECK(PhysicalDevice.createDevice(deviceCreateInfo));
    VULKAN_HPP_DEFAULT_DISPATCHER.init(Device);

    GraphicsQueue = Device.getQueue(m_Indices.graphics, 0);
    PresentQueue = Device.getQueue(m_Indices.present, 0);

    return true;
}

bool App::quit() {
    {
        vk::Result result = App::Device.waitIdle();
        if (result != vk::Result::eSuccess) {
            Logger::CRITICAL("[App] Failed to wait for device resources to be freed.");
        }
    }

    GlobalScene.clear();

    for (auto &[name, _] : m_Windows) {
        if (!getWindow(name)->destroy()) {
            Logger::ERROR("[App] Failed to destroy window.");
        }
    }

    m_Windows.clear();

    if (!Window::quit()) {
        Logger::ERROR("[App] Failed to destroy GLFW.");
        return false;
    }

#ifdef ENABLE_VALIDATION_LAYERS
    Instance.destroyDebugUtilsMessengerEXT(DebugMessenger);
    DebugMessenger = nullptr;
#endif

    Device.destroy();
    Device = nullptr;

    Instance.destroy();
    Instance = nullptr;

    return true;
}

std::shared_ptr<Window> App::createWindow(const std::string &name, const Window::Info &windowInfo, const glm::ivec2 &resolution) {
    m_Windows[name] = std::make_shared<Window>(windowInfo);

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
