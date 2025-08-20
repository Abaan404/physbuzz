#include "window.hpp"

#include "../app/application.hpp"
#include "../debug/logging.hpp"
#include "../events/window.hpp"

namespace Physbuzz {

static void glfwErrorCallback(int error, const char *description) {
    Logger::ERROR("[GLFW] ({}) {}", error, description);
}

Window::Window(const Info &info)
    : m_Info(info) {}

Window::operator GLFWwindow *() const {
    return m_Window;
}

bool Window::operator==(const Window &other) const {
    return m_Window == other.m_Window;
}

bool Window::init() {
    // error callback
    glfwSetErrorCallback(glfwErrorCallback);

    // init glfw
    int isInit = glfwInit();
    if (isInit == GLFW_FALSE) {
        Logger::ERROR("[Window] Failed to initialize the GLFW library.");
        return false;
    }

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

    return true;
}

bool Window::quit() {
    glfwTerminate();
    return true;
}

bool Window::build(const glm::ivec2 &resolution) {
    if (m_Window != nullptr) {
        Logger::ERROR("[Window] Could not create a constructed window.");
        return false;
    }

    m_Window = glfwCreateWindow(resolution.x, resolution.y, m_Info.title.c_str(), nullptr, nullptr);
    if (m_Window == nullptr) {
        Logger::ERROR("[Window] Could not create a GLFWwindow.");
        return false;
    }

    // point to here for window callbacks
    glfwSetWindowUserPointer(m_Window, this);

    glfwSetKeyCallback(m_Window, [](GLFWwindow *window, int key, int scancode, int action, int mods) {
        Window *appWindow = reinterpret_cast<Window *>(glfwGetWindowUserPointer(window));
        appWindow->notifyCallbacks<KeyEvent>({.window = appWindow, .key = static_cast<Key>(key), .scancode = scancode, .action = static_cast<Action>(action), .mods = static_cast<Modifier>(mods)});
    });

    glfwSetCursorEnterCallback(m_Window, [](GLFWwindow *window, int entered) {
        Window *appWindow = reinterpret_cast<Window *>(glfwGetWindowUserPointer(window));
        appWindow->notifyCallbacks<MouseEnteredEvent>({.window = appWindow, .entered = (entered == GLFW_TRUE)});
    });

    glfwSetScrollCallback(m_Window, [](GLFWwindow *window, double xoffset, double yoffset) {
        Window *appWindow = reinterpret_cast<Window *>(glfwGetWindowUserPointer(window));
        appWindow->notifyCallbacks<MouseScrollEvent>({.window = appWindow, .offset = {xoffset, yoffset}});
    });

    glfwSetCursorPosCallback(m_Window, [](GLFWwindow *window, double xpos, double ypos) {
        Window *appWindow = reinterpret_cast<Window *>(glfwGetWindowUserPointer(window));
        appWindow->notifyCallbacks<MousePositionEvent>({.window = appWindow, .position = {xpos, ypos}});
    });

    glfwSetMouseButtonCallback(m_Window, [](GLFWwindow *window, int button, int action, int mods) {
        Window *appWindow = reinterpret_cast<Window *>(glfwGetWindowUserPointer(window));
        appWindow->notifyCallbacks<MouseButtonEvent>({.window = appWindow, .button = static_cast<Button>(button), .action = static_cast<Action>(action), .mods = static_cast<Modifier>(mods)});
    });

    glfwSetFramebufferSizeCallback(m_Window, [](GLFWwindow *window, int width, int height) {
        Window *appWindow = reinterpret_cast<Window *>(glfwGetWindowUserPointer(window));
        appWindow->m_FramebufferResized = true;
        appWindow->notifyCallbacks<WindowResizeEvent>({.window = appWindow, .resolution = {width, height}});
    });

    glfwSetWindowCloseCallback(m_Window, [](GLFWwindow *window) {
        Window *appWindow = reinterpret_cast<Window *>(glfwGetWindowUserPointer(window));
        appWindow->notifyCallbacks<WindowCloseEvent>({.window = appWindow});
        appWindow->close(); // close the window when glfw requests it
    });

    glfwSetCharCallback(m_Window, [](GLFWwindow *window, unsigned int codepoint) {
        Window *appWindow = reinterpret_cast<Window *>(glfwGetWindowUserPointer(window));
        appWindow->notifyCallbacks<CharEvent>({.window = appWindow, .codepoint = codepoint});
    });

    glfwSetWindowMaximizeCallback(m_Window, [](GLFWwindow *window, int maximized) {
        Window *appWindow = reinterpret_cast<Window *>(glfwGetWindowUserPointer(window));
        appWindow->notifyCallbacks<WindowMaximizeEvent>({.window = appWindow, .maximized = static_cast<bool>(maximized & GLFW_MAXIMIZED)});
    });

    glfwSetDropCallback(m_Window, [](GLFWwindow *window, int path_count, const char *paths[]) {
        Window *appWindow = reinterpret_cast<Window *>(glfwGetWindowUserPointer(window));
        appWindow->notifyCallbacks<MouseDropEvent>({.window = appWindow, .paths = {paths, paths + path_count}});
    });

    glfwSetWindowPosCallback(m_Window, [](GLFWwindow *window, int xpos, int ypos) {
        Window *appWindow = reinterpret_cast<Window *>(glfwGetWindowUserPointer(window));
        appWindow->notifyCallbacks<WindowPositionEvent>({.window = appWindow, .position = {xpos, ypos}});
    });

    glfwSetWindowRefreshCallback(m_Window, [](GLFWwindow *window) {
        Window *appWindow = reinterpret_cast<Window *>(glfwGetWindowUserPointer(window));
        appWindow->notifyCallbacks<WindowRefreshEvent>({.window = appWindow});
    });

    glfwSetWindowFocusCallback(m_Window, [](GLFWwindow *window, int focused) {
        Window *appWindow = reinterpret_cast<Window *>(glfwGetWindowUserPointer(window));
        appWindow->notifyCallbacks<WindowFocusEvent>({.window = appWindow, .focused = static_cast<bool>(focused & GLFW_FOCUSED)});
    });

    glfwSetWindowIconifyCallback(m_Window, [](GLFWwindow *window, int iconified) {
        Window *appWindow = reinterpret_cast<Window *>(glfwGetWindowUserPointer(window));
        appWindow->notifyCallbacks<WindowIconifyEvent>({.window = appWindow, .iconified = static_cast<bool>(iconified & GLFW_ICONIFIED)});
    });

    // create a surface
    VkSurfaceKHR surface;
    glfwCreateWindowSurface(App::Instance, m_Window, nullptr, &surface);
    m_Surface = surface;

    if (!PBZ_VK_CHECK(App::PhysicalDevice.getSurfaceSupportKHR(App::Indices.present, m_Surface))) {
        Logger::CRITICAL("[App] Graphics and present queue indices do not match, submit a bug report.");
        destroy();
        return false;
    }

    // create a swapchain
    buildSwapChain();

    return true;
}

bool Window::destroy() {
    if (m_Window == nullptr) {
        Logger::ERROR("[Window] Could not destroy a destructed window.");
        return false;
    }

    destroySwapChain();

    App::Instance.destroySurfaceKHR(m_Surface);
    m_Surface = nullptr;

    close();

    glfwDestroyWindow(m_Window);
    m_Window = nullptr;

    return true;
}

void Window::close() const {
    glfwSetWindowShouldClose(m_Window, GLFW_TRUE);
}

void Window::setTitle(const std::string &title) const {
    glfwSetWindowTitle(m_Window, title.c_str());
}

void Window::setPos(const glm::ivec2 &position) const {
    glfwSetWindowPos(m_Window, position.x, position.y);
}

void Window::iconify() const {
    glfwIconifyWindow(m_Window);
}

void Window::restore() const {
    glfwRestoreWindow(m_Window);
}

void Window::maximize() const {
    glfwMaximizeWindow(m_Window);
}

void Window::poll() const {
    glfwPollEvents();
}

void Window::setCursorPos(const glm::ivec2 &position) {
    glfwSetCursorPos(m_Window, position.x, position.y);
}

const glm::ivec2 Window::getResolution() const {
    int width;
    int height;

    glfwGetFramebufferSize(m_Window, &width, &height);
    return glm::ivec2(width, height);
}

const glm::dvec2 Window::getCursorPos() const {
    double xpos, ypos;

    glfwGetCursorPos(m_Window, &xpos, &ypos);
    return glm::dvec2(xpos, ypos);
}

bool Window::shouldClose() const {
    return glfwWindowShouldClose(m_Window);
}

void Window::setResolution(const glm::ivec2 &resolution) {
    glfwSetWindowSize(m_Window, resolution.x, resolution.y);
}

const Window::Info &Window::getInfo() const {
    return m_Info;
}

void Window::setCursorCapture(bool capture) const {
    if (capture) {
        glfwSetInputMode(m_Window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    } else {
        glfwSetInputMode(m_Window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    }
}

std::vector<const char *> Window::requiredExtensions() {
    // Get the required instance extensions from GLFW.
    std::uint32_t glfwExtensionCount = 0;
    const char **glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

    return {glfwExtensions, glfwExtensions + glfwExtensionCount};
}

void Window::buildSwapChain() {
    if (m_SwapChain != nullptr) {
        Logger::WARNING("[Window] trying to build a constructed swapchain.");
    }

    // get device info
    vk::SurfaceCapabilitiesKHR surfaceCapabilities = PBZ_VK_CHECK(App::PhysicalDevice.getSurfaceCapabilitiesKHR(m_Surface));
    std::vector<vk::SurfaceFormatKHR> availableFormats = PBZ_VK_CHECK(App::PhysicalDevice.getSurfaceFormatsKHR(m_Surface));
    std::vector<vk::PresentModeKHR> availablePresentModes = PBZ_VK_CHECK(App::PhysicalDevice.getSurfacePresentModesKHR(m_Surface));

    // setup format
    {
        auto it = std::find_if(availableFormats.begin(), availableFormats.end(), [&](const vk::SurfaceFormatKHR &format) {
            return format.format == m_Info.swapChain.format && format.colorSpace == m_Info.swapChain.colorSpace;
        });

        vk::SurfaceFormatKHR surfaceFormat = (it != availableFormats.end()) ? *it : availableFormats.front();
        m_Info.swapChain.format = surfaceFormat.format;
        m_Info.swapChain.colorSpace = surfaceFormat.colorSpace;
    }

    // and present mode
    {
        auto it = std::find_if(availablePresentModes.begin(), availablePresentModes.end(), [&](vk::PresentModeKHR presentMode) {
            return presentMode == m_Info.swapChain.presentMode;
        });

        m_Info.swapChain.presentMode = (it != availablePresentModes.end()) ? *it : vk::PresentModeKHR::eFifo;
    }

    std::uint32_t minImageCount = std::max(3u, surfaceCapabilities.minImageCount);
    if (surfaceCapabilities.maxImageCount > 0 && minImageCount > surfaceCapabilities.maxImageCount) {
        minImageCount = surfaceCapabilities.maxImageCount;
    }

    glm::ivec2 resolution = getResolution();
    vk::SwapchainCreateInfoKHR swapChainCreateInfo = {
        .flags = vk::SwapchainCreateFlagsKHR(),
        .surface = m_Surface,
        .minImageCount = minImageCount,
        .imageFormat = m_Info.swapChain.format,
        .imageColorSpace = m_Info.swapChain.colorSpace,
        .imageExtent = {static_cast<std::uint32_t>(resolution.x), static_cast<std::uint32_t>(resolution.y)},
        .imageArrayLayers = 1,
        .imageUsage = vk::ImageUsageFlagBits::eColorAttachment,
        .imageSharingMode = vk::SharingMode::eExclusive,
        .preTransform = surfaceCapabilities.currentTransform,
        .compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque,
        .presentMode = m_Info.swapChain.presentMode,
        .clipped = true,
        .oldSwapchain = nullptr,
    };

    m_SwapChain = PBZ_VK_CHECK(App::Device.createSwapchainKHR(swapChainCreateInfo));
    m_SwapChainImages = PBZ_VK_CHECK(App::Device.getSwapchainImagesKHR(m_SwapChain));

    vk::ImageViewCreateInfo imageViewCreateInfo = {
        .viewType = vk::ImageViewType::e2D,
        .format = m_Info.swapChain.format,
        .subresourceRange = {
            .aspectMask = vk::ImageAspectFlagBits::eColor,
            .baseMipLevel = 0,
            .levelCount = 1,
            .baseArrayLayer = 0,
            .layerCount = 1,
        },
    };

    for (const auto &image : m_SwapChainImages) {
        imageViewCreateInfo.image = image;
        m_SwapChainImageViews.push_back(PBZ_VK_CHECK(App::Device.createImageView(imageViewCreateInfo)));
    }
}

void Window::destroySwapChain() {
    if (m_SwapChain == nullptr) {
        Logger::WARNING("[Window] trying to destroy a destructed swapchain.");
    }

    for (const auto &image : m_SwapChainImageViews) {
        App::Device.destroyImageView(image);
    }

    App::Device.destroySwapchainKHR(m_SwapChain);
    m_SwapChain = nullptr;

    m_SwapChainImageViews.clear();
    m_SwapChainImages.clear();
}

void Window::recreateSwapChain() {
    std::int32_t width = 0, height = 0;
    glfwGetFramebufferSize(m_Window, &width, &height);
    while (width == 0 || height == 0) {
        glfwGetFramebufferSize(m_Window, &width, &height);
        glfwWaitEvents();
    }

    // wait for resources to be ready
    // Note: It is possible to create a new swap chain while drawing commands on an image from the old swap chain are still in-flight. You need to pass the previous swap chain to the oldSwapchain field in the VkSwapchainCreateInfoKHR struct and destroy the old swap chain as soon as you’ve finished using it. (https://docs.vulkan.org/tutorial/latest/03_Drawing_a_triangle/04_Swap_chain_recreation.html#_recreating_the_swap_chain)
    {
        vk::Result result = App::Device.waitIdle();
        if (result != vk::Result::eSuccess) {
            Logger::CRITICAL("[Window] Failed to wait for device resources to be freed.");
        }
    }

    destroySwapChain();
    buildSwapChain();
}

} // namespace Physbuzz
