#include "application.hpp"
#include <algorithm>
#include <array>
#include <glm/common.hpp>
#include <map>
#include <vulkan/vulkan_enums.hpp>

#ifndef NDEBUG
#define ENABLE_VALIDATION_LAYERS
#endif

VULKAN_HPP_DEFAULT_DISPATCH_LOADER_DYNAMIC_STORAGE

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

namespace Physbuzz {

bool App::build() {
    VULKAN_HPP_DEFAULT_DISPATCHER.init();

    // setup logging
    Logger::build();

    // init glfw
    if (!Window::init()) {
        Logger::CRITICAL("[App] Failed to setup windowing.");
        return false;
    }

    // Get the required instance extensions from GLFW.
    std::vector<const char *> extensions = Window::requiredExtensions();

#ifdef ENABLE_VALIDATION_LAYERS
    extensions.emplace_back(vk::EXTDebugUtilsExtensionName);
#endif

    // Check if the required GLFW extensions are supported by the Vulkan implementation.
    auto [extensionPropertiesResult, extensionProperties] = vk::enumerateInstanceExtensionProperties();
    if (extensionPropertiesResult != vk::Result::eSuccess) {
        Logger::CRITICAL("[App] Could not query loaded vulkan extensions.");
        return false;
    }

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
        .applicationVersion = VK_MAKE_API_VERSION(0, 0, 1, 0),
        .pEngineName = "Physbuzz",
        .engineVersion = VK_MAKE_API_VERSION(0, 0, 3, 0),
        .apiVersion = VK_API_VERSION_1_0,
    };

    vk::InstanceCreateInfo createInfo = {
        .pApplicationInfo = &appInfo,
        .enabledExtensionCount = static_cast<std::uint32_t>(extensions.size()),
        .ppEnabledExtensionNames = extensions.data(),
    };

    if (vk::createInstance(&createInfo, nullptr, &m_Instance) != vk::Result::eSuccess) {
        Logger::ERROR("[App] Failed to create vulkan instance.");
        return false;
    }

    VULKAN_HPP_DEFAULT_DISPATCHER.init(m_Instance);

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

    auto [debugMessengerResult, debugMessenger] = m_Instance.createDebugUtilsMessengerEXT(debugUtilsMessengerCreateInfoEXT);
    if (debugMessengerResult != vk::Result::eSuccess) {
        Logger::WARNING("[App] Failed to load debug validation layers.");
    } else {
        m_DebugMessanger = debugMessenger;
    }
#endif

    // get the device
    auto [devicesResult, devices] = m_Instance.enumeratePhysicalDevices();

    if (devicesResult != vk::Result::eSuccess) {
        Logger::CRITICAL("[App] Could not query physical vulkan devices.");
        return false;
    }

    if (devices.empty()) {
        Logger::CRITICAL("[App] No GPU supporting Vulkan found.");
        return false;
    }

    std::multimap<int, vk::PhysicalDevice> deviceCandidates;

    // find an appropriate device
    for (const auto &device : devices) {
        auto deviceProperties = device.getProperties();
        uint32_t score = 0;

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

    auto [score, physicalDevice] = *deviceCandidates.rbegin();
    if (score <= 0) {
        Logger::CRITICAL("[App] failed to find a suitable GPU!.");
        return false;
    }

    m_PhysicalDevice = physicalDevice;

    // find the needed queues
    std::vector<vk::QueueFamilyProperties> queueFamilyProperties = m_PhysicalDevice.getQueueFamilyProperties();

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
            m_PhysicalDevice.getFeatures2(),
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
        .queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size()),
        .pQueueCreateInfos = queueCreateInfos.data(),
        .enabledExtensionCount = static_cast<std::uint32_t>(deviceExtensions.size()),
        .ppEnabledExtensionNames = deviceExtensions.data(),
    };

    auto [deviceResult, device] = m_PhysicalDevice.createDevice(deviceCreateInfo);

    if (deviceResult != vk::Result::eSuccess) {
        Logger::CRITICAL("[App] Failed to create a logical vulkan device.");
        return false;
    }

    m_Device = device;
    m_GraphicsQueue = m_Device.getQueue(m_Indices.graphics, 0);
    m_PresentQueue = m_Device.getQueue(m_Indices.present, 0);

    return true;
}

bool App::destroy() {
    m_Scene.clear();

    for (auto &window : m_Windows) {
        window->destroy();
    }

    if (!Window::terminate()) {
        Logger::ERROR("[App] Failed to terminate windowing.");
        return false;
    }

    m_Device.destroy();
    m_Instance.destroy();

    return true;
}

std::shared_ptr<Window> App::createWindow(const Window::Info &windowInfo, const glm::ivec2 &resolution) {
    m_Windows.emplace_back(std::make_shared<Window>(windowInfo));

    std::shared_ptr<Window> &window = m_Windows.back();
    if (!window->build(m_Instance, resolution)) {
        Logger::CRITICAL("[App] Could not create a window.");
        return nullptr;
    }

    auto [surfaceSupportResult, surfaceSupport] = m_PhysicalDevice.getSurfaceSupportKHR(m_Indices.present, window->m_Surface);

    // test queue
    if (surfaceSupportResult != vk::Result::eSuccess) {
        Logger::CRITICAL("[App] Could not query present queue support.");
        destroyWindow(window);
        return nullptr;
    }

    if (!surfaceSupport) {
        Logger::CRITICAL("[App] Graphics and present queue indices do not match, submit a bug report.");
        destroyWindow(window);
        return nullptr;
    }

    // setup swapchain
    auto [surfaceCapabilitiesResult, surfaceCapabilities] = m_PhysicalDevice.getSurfaceCapabilitiesKHR(window->m_Surface);
    if (surfaceCapabilitiesResult != vk::Result::eSuccess) {
        Logger::CRITICAL("[App] Could not query surface capabilities.");
        destroyWindow(window);
        return nullptr;
    }

    auto [availableFormatsResult, availableFormats] = m_PhysicalDevice.getSurfaceFormatsKHR(window->m_Surface);
    if (surfaceCapabilitiesResult != vk::Result::eSuccess) {
        Logger::CRITICAL("[App] Could not query available surface formats.");
        destroyWindow(window);
        return nullptr;
    }

    auto [availablePresentModesResult, availablePresentModes] = m_PhysicalDevice.getSurfacePresentModesKHR(window->m_Surface);
    if (availablePresentModesResult != vk::Result::eSuccess) {
        Logger::CRITICAL("[App] Could not query available surface modes.");
        destroyWindow(window);
        return nullptr;
    }

    // setup format
    {
        auto it = std::find_if(availableFormats.begin(), availableFormats.end(), [&](const vk::SurfaceFormatKHR &format) {
            return format.format == window->m_Info.swapChain.format && format.colorSpace == window->m_Info.swapChain.colorSpace;
        });

        vk::SurfaceFormatKHR surfaceFormat = (it != availableFormats.end()) ? *it : availableFormats.front();
        window->m_Info.swapChain.format = surfaceFormat.format;
        window->m_Info.swapChain.colorSpace = surfaceFormat.colorSpace;
    }

    // and present mode
    {
        auto it = std::find_if(availablePresentModes.begin(), availablePresentModes.end(), [&](vk::PresentModeKHR presentMode) {
            return presentMode == window->m_Info.swapChain.presentMode;
        });

        window->m_Info.swapChain.presentMode = (it != availablePresentModes.end()) ? *it : vk::PresentModeKHR::eFifo;
    }

    // glm::clamp()

    auto minImageCount = std::max(3u, surfaceCapabilities.minImageCount);
    if (surfaceCapabilities.maxImageCount > 0 && minImageCount > surfaceCapabilities.maxImageCount) {
        minImageCount = surfaceCapabilities.maxImageCount;
    }

    vk::SwapchainCreateInfoKHR swapChainCreateInfo = {
        .flags = vk::SwapchainCreateFlagsKHR(),
        .surface = window->m_Surface,
        .minImageCount = minImageCount,
        .imageFormat = window->m_Info.swapChain.format,
        .imageColorSpace = window->m_Info.swapChain.colorSpace,
        .imageExtent = {static_cast<std::uint32_t>(resolution.x), static_cast<std::uint32_t>(resolution.y)},
        .imageArrayLayers = 1,
        .imageUsage = vk::ImageUsageFlagBits::eColorAttachment,
        .imageSharingMode = vk::SharingMode::eExclusive,
        .preTransform = surfaceCapabilities.currentTransform,
        .compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque,
        .presentMode = window->m_Info.swapChain.presentMode,
        .clipped = true,
        .oldSwapchain = nullptr,
    };

    auto [swapChainResult, swapChain] = m_Device.createSwapchainKHR(swapChainCreateInfo);
    if (swapChainResult != vk::Result::eSuccess) {
        Logger::CRITICAL("[App] Failed to create swapchain.");
        destroyWindow(window);
        return nullptr;
    }

    window->m_SwapChain = swapChain;
    window->m_SwapChainViews.clear();

    vk::ImageViewCreateInfo imageViewCreateInfo = {
        .viewType = vk::ImageViewType::e2D,
        .format = window->m_Info.swapChain.format,
        .subresourceRange = {
            .aspectMask = vk::ImageAspectFlagBits::eColor,
            .baseMipLevel = 0,
            .levelCount = 1,
            .baseArrayLayer = 0,
            .layerCount = 1,
        },
    };

    auto [swapChainImagesResult, swapChainImages] = m_Device.getSwapchainImagesKHR(window->m_SwapChain);
    if (swapChainImagesResult != vk::Result::eSuccess) {
        Logger::CRITICAL("[App] Could not query swapchain images.");
        destroyWindow(window);
        return nullptr;
    }

    for (auto image : swapChainImages) {
        imageViewCreateInfo.image = image;
        auto [imageViewResult, imageView] = m_Device.createImageView(imageViewCreateInfo);

        if (imageViewResult != vk::Result::eSuccess) {
            Logger::CRITICAL("[App] Could not query swapchain images view.");
            destroyWindow(window);
            return nullptr;
        }

        window->m_SwapChainViews.push_back(imageView);
    }

    return window;
}

bool App::destroyWindow(const std::shared_ptr<Window> &window) {
    auto windowIt = std::find(m_Windows.begin(), m_Windows.end(), window);
    if (windowIt == m_Windows.end()) {
        return false;
    }

    window->m_SwapChainViews.clear();
    window->destroy();

    m_Windows.erase(windowIt);
    return true;
}

const std::list<std::shared_ptr<Window>> &App::getWindows() {
    return m_Windows;
}

Scene &App::getGlobalScene() {
    return m_Scene;
}

} // namespace Physbuzz
