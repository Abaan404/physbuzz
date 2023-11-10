#include "vulkan.hpp"
#include <vulkan/vulkan_core.h>

#define VULKAN_DEBUG_REPORT

#ifdef VULKAN_DEBUG_REPORT
static VKAPI_ATTR VkBool32 VKAPI_CALL debug_report(VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT objectType, uint64_t object, size_t location, int32_t messageCode, const char *pLayerPrefix, const char *pMessage, void *pUserData) {
    (void)flags;
    (void)object;
    (void)location;
    (void)messageCode;
    (void)pUserData;
    (void)pLayerPrefix; // Unused arguments
    fprintf(stderr, "[vulkan] Debug report from ObjectType: %i\nMessage: %s\n\n", objectType, pMessage);
    return VK_FALSE;
}
#endif // VULKAN_DEBUG_REPORT

// ref: imgui vulkan-sdl2 example
// could have tried to do this myself but
// there is so much boilerplate, im better
// off copying imgui's bootstrap example and
// returning to this if something breaks

// psa learning how to setup vulkan at 3am is not a good idea

// void VulkanContext::frame_render(ImGui_ImplVulkanH_Window *wd, ImDrawData *draw_data) {
//     VkResult err;
//
//     VkSemaphore image_acquired_semaphore = wd->FrameSemaphores[wd->SemaphoreIndex].ImageAcquiredSemaphore;
//     VkSemaphore render_complete_semaphore = wd->FrameSemaphores[wd->SemaphoreIndex].RenderCompleteSemaphore;
//     err = vkAcquireNextImageKHR(device, wd->Swapchain, UINT64_MAX, image_acquired_semaphore, VK_NULL_HANDLE, &wd->FrameIndex);
//     if (err == VK_ERROR_OUT_OF_DATE_KHR || err == VK_SUBOPTIMAL_KHR) {
//         swapchain_rebuild = true;
//         return;
//     }
//     VK_CHECK(err);
//
//     ImGui_ImplVulkanH_Frame *fd = &wd->Frames[wd->FrameIndex];
//     {
//         VK_CHECK(vkWaitForFences(device, 1, &fd->Fence, VK_TRUE, UINT64_MAX)); // wait indefinitely instead of periodically checking
//         VK_CHECK(vkResetFences(device, 1, &fd->Fence));
//     }
//     {
//         VK_CHECK(vkResetCommandPool(device, fd->CommandPool, 0));
//         VkCommandBufferBeginInfo info = {};
//         info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
//         info.flags |= VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
//         VK_CHECK(vkBeginCommandBuffer(fd->CommandBuffer, &info));
//     }
//     {
//         VkRenderPassBeginInfo info = {};
//         info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
//         info.renderPass = wd->RenderPass;
//         info.framebuffer = fd->Framebuffer;
//         info.renderArea.extent.width = wd->Width;
//         info.renderArea.extent.height = wd->Height;
//         info.clearValueCount = 1;
//         info.pClearValues = &wd->ClearValue;
//         vkCmdBeginRenderPass(fd->CommandBuffer, &info, VK_SUBPASS_CONTENTS_INLINE);
//     }
//
//     // Record dear imgui primitives into command buffer
//     ImGui_ImplVulkan_RenderDrawData(draw_data, fd->CommandBuffer);
//
//     // Submit command buffer
//     vkCmdEndRenderPass(fd->CommandBuffer);
//     {
//         VkPipelineStageFlags wait_stage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
//         VkSubmitInfo info = {};
//         info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
//         info.waitSemaphoreCount = 1;
//         info.pWaitSemaphores = &image_acquired_semaphore;
//         info.pWaitDstStageMask = &wait_stage;
//         info.commandBufferCount = 1;
//         info.pCommandBuffers = &fd->CommandBuffer;
//         info.signalSemaphoreCount = 1;
//         info.pSignalSemaphores = &render_complete_semaphore;
//
//         VK_CHECK(vkEndCommandBuffer(fd->CommandBuffer));
//         VK_CHECK(vkQueueSubmit(queue, 1, &info, fd->Fence));
//     }
// }

void VulkanContext::frame_present() {
    VkResult err;
    if (swapchain_rebuild)
        return;
    VkSemaphore render_complete_semaphore = main_window_data.FrameSemaphores[main_window_data.SemaphoreIndex].RenderCompleteSemaphore;
    VkPresentInfoKHR info = {};
    info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    info.waitSemaphoreCount = 1;
    info.pWaitSemaphores = &render_complete_semaphore;
    info.swapchainCount = 1;
    info.pSwapchains = &main_window_data.Swapchain;
    info.pImageIndices = &main_window_data.FrameIndex;
    err = vkQueuePresentKHR(queue, &info);
    if (err == VK_ERROR_OUT_OF_DATE_KHR || err == VK_SUBOPTIMAL_KHR) {
        swapchain_rebuild = true;
        return;
    }
    VK_CHECK(err);
    main_window_data.SemaphoreIndex = (main_window_data.SemaphoreIndex + 1) % main_window_data.ImageCount; // Now we can use the next set of semaphores
}

// void VulkanContext::rebuild_swapchain(int width, int height) {
//     ImGui_ImplVulkan_SetMinImageCount(min_image_count);
//     ImGui_ImplVulkanH_CreateOrResizeWindow(instance, physical_device, device, &main_window_data, queue_family, allocator, width, height, min_image_count);
//     main_window_data.FrameIndex = 0;
//     swapchain_rebuild = false;
// }

void VulkanContext::cleanup() {
    vkDestroyDevice(device, allocator);

#ifdef VULKAN_DEBUG_REPORT
    // Remove the debug report callback
    auto vkDestroyDebugReportCallbackEXT = (PFN_vkDestroyDebugReportCallbackEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugReportCallbackEXT");
    vkDestroyDebugReportCallbackEXT(instance, debug_report, allocator);
#endif

    vkDestroyInstance(instance, allocator);
    vkDestroyDescriptorPool(device, descriptor_pool, allocator);
}

void VulkanBuilder::setup() {
    uint32_t extensions_count = 0;
    SDL_Vulkan_GetInstanceExtensions(window, &extensions_count, nullptr);
    std::vector<const char *> instance_extensions(extensions_count);
    SDL_Vulkan_GetInstanceExtensions(window, &extensions_count, instance_extensions.data());

    // create the VkInstance here
    create_instance(instance_extensions);

    // Select the GPU here
    vk_context->physical_device = select_physical_device();
    select_graphics_queue_family();

    // Setup logical device and queue here
    std::vector<const char *> device_extensions = {"VK_KHR_swapchain"};
    select_device(device_extensions);

    // Create a vulkan surface and swapchain
    create_vulkan_surface();
    create_swapchain(); // imgui itself creates a swapchain

    // framebuffers for SDL and ImGui
    // create_frame_buffers();

    // honestly dont know what this does but ImGui needs it
    // create_descriptor_pool();
}

void VulkanBuilder::create_vulkan_surface() {
    // Create Window Surface
    if (SDL_Vulkan_CreateSurface(window, vk_context->instance, &vk_context->surface) == 0) {
        printf("[ERROR] Failed to create Vulkan surface.\n");
        exit(1);
    }
}

// void VulkanBuilder::create_frame_buffers() {
//     int w, h;
//     SDL_GetWindowSize(window, &w, &h);
//     ImGui_ImplVulkanH_Window *wd = &vk_context->main_window_data;
//     setup_vulkan_window(wd, w, h);
// }

// void VulkanBuilder::setup_vulkan_window(ImGui_ImplVulkanH_Window *window_data, int width, int height) {
//     window_data->Surface = vk_context->surface;
//
//     // Check for WSI support
//     VkBool32 res;
//     vkGetPhysicalDeviceSurfaceSupportKHR(vk_context->physical_device, vk_context->queue_family, window_data->Surface, &res);
//     if (res != VK_TRUE) {
//         fprintf(stderr, "Error no WSI support on physical device 0\n");
//         exit(-1);
//     }
//
//     // Select Surface Format
//     const VkFormat requestSurfaceImageFormat[] = {VK_FORMAT_B8G8R8A8_UNORM, VK_FORMAT_R8G8B8A8_UNORM, VK_FORMAT_B8G8R8_UNORM, VK_FORMAT_R8G8B8_UNORM};
//     const VkColorSpaceKHR requestSurfaceColorSpace = VK_COLORSPACE_SRGB_NONLINEAR_KHR;
//     window_data->SurfaceFormat = ImGui_ImplVulkanH_SelectSurfaceFormat(vk_context->physical_device, window_data->Surface, requestSurfaceImageFormat, (size_t)IM_ARRAYSIZE(requestSurfaceImageFormat), requestSurfaceColorSpace);
//
//     // Select Present Mode
// #ifdef IMGUI_UNLIMITED_FRAME_RATE
//     VkPresentModeKHR present_modes[] = {VK_PRESENT_MODE_MAILBOX_KHR, VK_PRESENT_MODE_IMMEDIATE_KHR, VK_PRESENT_MODE_FIFO_KHR};
// #else
//     VkPresentModeKHR present_modes[] = {VK_PRESENT_MODE_FIFO_KHR};
// #endif
//     window_data->PresentMode = ImGui_ImplVulkanH_SelectPresentMode(vk_context->physical_device, window_data->Surface, &present_modes[0], IM_ARRAYSIZE(present_modes));
//     // printf("[vulkan] Selected PresentMode = %d\n", wd->PresentMode);
//
//     // Create SwapChain, RenderPass, Framebuffer, etc.
//     if (vk_context->min_image_count < 2) {
//         printf("[ERROR] Invalid image counts\n");
//         exit(1);
//     }
//
//     ImGui_ImplVulkanH_CreateOrResizeWindow(vk_context->instance, vk_context->physical_device, vk_context->device, window_data, vk_context->queue_family, vk_context->allocator, width, height, vk_context->min_image_count);
// }

bool VulkanBuilder::is_extension_available(const std::vector<VkExtensionProperties> &properties, const char *extension) {
    for (const VkExtensionProperties &p : properties)
        if (strcmp(p.extensionName, extension) == 0)
            return true;
    return false;
}

void VulkanBuilder::create_instance(std::vector<const char *> instance_extensions) {
    VkInstanceCreateInfo create_info = {};

    // Enumerate available extensions
    uint32_t properties_count;
    VK_CHECK(vkEnumerateInstanceExtensionProperties(nullptr, &properties_count, nullptr));
    std::vector<VkExtensionProperties> properties(properties_count);
    VK_CHECK(vkEnumerateInstanceExtensionProperties(nullptr, &properties_count, properties.data()));

    // Enable required extensions
    if (is_extension_available(properties, VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME))
        instance_extensions.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);

        // #ifdef VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME
        //     if (is_extension_available(properties, VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME)) {
        //         instance_extensions.push_back(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);
        //         create_info.flags |= VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
        //     }
        // #endif

#ifdef VULKAN_DEBUG_REPORT
    const char *layers[] = {"VK_LAYER_KHRONOS_validation"};
    create_info.enabledLayerCount = 1;
    create_info.ppEnabledLayerNames = layers;
    instance_extensions.push_back("VK_EXT_debug_report");
#endif

    // Create Vulkan Instance
    create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    create_info.enabledExtensionCount = (uint32_t)instance_extensions.size();
    create_info.ppEnabledExtensionNames = instance_extensions.data();
    VK_CHECK(vkCreateInstance(&create_info, vk_context->allocator, &vk_context->instance));

#ifdef VULKAN_DEBUG_REPORT
    auto vkCreateDebugReportCallbackEXT = (PFN_vkCreateDebugReportCallbackEXT)vkGetInstanceProcAddr(vk_context->instance, "vkCreateDebugReportCallbackEXT");
    IM_ASSERT(vkCreateDebugReportCallbackEXT != nullptr);
    VkDebugReportCallbackCreateInfoEXT debug_report_ci = {};
    debug_report_ci.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
    debug_report_ci.flags = VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT | VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT;
    debug_report_ci.pfnCallback = debug_report;
    debug_report_ci.pUserData = nullptr;
    VK_CHECK(vkCreateDebugReportCallbackEXT(vk_context->instance, &debug_report_ci, vk_context->allocator, &vk_context->debug_report));
#endif
}

VkPhysicalDevice VulkanBuilder::select_physical_device() {
    uint32_t gpu_count;
    VK_CHECK(vkEnumeratePhysicalDevices(vk_context->instance, &gpu_count, nullptr));
    if (gpu_count <= 0) {
        printf("[ERROR] No GPUs found\n");
        exit(1);
    }

    std::vector<VkPhysicalDevice> gpus(gpu_count);
    VK_CHECK(vkEnumeratePhysicalDevices(vk_context->instance, &gpu_count, gpus.data()));

    // If a number >1 of GPUs got reported, find discrete GPU if present, or use first one available. This covers
    // most common cases (multi-gpu/integrated+dedicated graphics). Handling more complicated setups (multiple
    // dedicated GPUs) is out of scope of this sample.
    for (VkPhysicalDevice &device : gpus) {
        VkPhysicalDeviceProperties properties;
        vkGetPhysicalDeviceProperties(device, &properties);
        if (properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
            return device;
    }

    // Use first GPU (Integrated) is a Discrete one is not available.
    if (gpu_count > 0)
        return gpus[0];

    return VK_NULL_HANDLE;
}

void VulkanBuilder::select_graphics_queue_family() {
    uint32_t count;
    vkGetPhysicalDeviceQueueFamilyProperties(vk_context->physical_device, &count, nullptr);
    VkQueueFamilyProperties *queues = new VkQueueFamilyProperties[count];
    vkGetPhysicalDeviceQueueFamilyProperties(vk_context->physical_device, &count, queues);

    for (uint32_t i = 0; i < count; i++)
        if (queues[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            vk_context->queue_family = i;
            break;
        }

    delete[] queues;

    if (vk_context->queue_family == (uint32_t)-1) {
        printf("[ERROR] Could not select a graphics queue familiy\n");
        exit(1);
    }
}

void VulkanBuilder::select_device(std::vector<const char *> device_extensions) {
    device_extensions.push_back("VK_KHR_swapchain");

    // Enumerate physical device extension
    uint32_t properties_count;
    vkEnumerateDeviceExtensionProperties(vk_context->physical_device, nullptr, &properties_count, nullptr);
    std::vector<VkExtensionProperties> properties(properties_count);
    vkEnumerateDeviceExtensionProperties(vk_context->physical_device, nullptr, &properties_count, properties.data());

#ifdef VK_KHR_PORTABILITY_SUBSET_EXTENSION_NAME
    if (IsExtensionAvailable(properties, VK_KHR_PORTABILITY_SUBSET_EXTENSION_NAME))
        device_extensions.push_back(VK_KHR_PORTABILITY_SUBSET_EXTENSION_NAME);
#endif

    const float queue_priority[] = {1.0f};
    VkDeviceQueueCreateInfo queue_info[1] = {};
    queue_info[0].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queue_info[0].queueFamilyIndex = vk_context->queue_family;
    queue_info[0].queueCount = 1;
    queue_info[0].pQueuePriorities = queue_priority;
    VkDeviceCreateInfo create_info = {};
    create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    create_info.queueCreateInfoCount = sizeof(queue_info) / sizeof(queue_info[0]);
    create_info.pQueueCreateInfos = queue_info;
    create_info.enabledExtensionCount = (uint32_t)device_extensions.size();
    create_info.ppEnabledExtensionNames = device_extensions.data();

    // create the logical device context
    VK_CHECK(vkCreateDevice(vk_context->physical_device, &create_info, vk_context->allocator, &vk_context->device));

    // get the device queue
    vkGetDeviceQueue(vk_context->device, vk_context->queue_family, 0, &vk_context->queue);
}

void VulkanBuilder::create_swapchain() {
    uint32_t format_count = 0;
    vkGetPhysicalDeviceSurfaceFormatsKHR(vk_context->physical_device, vk_context->surface, &format_count, 0);
    std::vector<VkSurfaceFormatKHR> surface_formats(format_count);
    vkGetPhysicalDeviceSurfaceFormatsKHR(vk_context->physical_device, vk_context->surface, &format_count, surface_formats.data());

    for (uint32_t i = 0; i < format_count; i++) {
        VkSurfaceFormatKHR format = surface_formats[i];
        if (format.format == VK_FORMAT_B8G8R8A8_SRGB) {
            vk_context->surface_format = format;
            break;
        }
    }

    VkSurfaceCapabilitiesKHR surface_capabilities;
    VK_CHECK(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(vk_context->physical_device, vk_context->surface, &surface_capabilities));

    uint32_t img_count = surface_capabilities.minImageCount + 1;
    if (img_count > surface_capabilities.maxImageCount) --img_count;

    VkSwapchainCreateInfoKHR swapchain_info = {
        .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
        .surface = vk_context->surface,
        .minImageCount = img_count,
        .imageFormat = vk_context->surface_format.format,
        .imageExtent = surface_capabilities.currentExtent,
        .imageArrayLayers = 1,
        .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
        .preTransform = surface_capabilities.currentTransform,
        // setting opaque for now, might need to return here later
        .compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
    };

    VK_CHECK(vkCreateSwapchainKHR(vk_context->device, &swapchain_info, vk_context->allocator, &vk_context->swapchain));
}

void VulkanBuilder::create_descriptor_pool() {
    VkResult error;
    VkDescriptorPoolSize pool_sizes[] =
        {
            {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1},
        };
    VkDescriptorPoolCreateInfo pool_info = {};
    pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
    pool_info.maxSets = 1;
    pool_info.poolSizeCount = (uint32_t)(sizeof(pool_sizes) / sizeof(VkDescriptorPoolSize));
    pool_info.pPoolSizes = pool_sizes;
    VK_CHECK(vkCreateDescriptorPool(vk_context->device, &pool_info, vk_context->allocator, &vk_context->descriptor_pool));
}
