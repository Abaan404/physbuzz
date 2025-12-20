#include "imgui_impl_physbuzz.hpp"

#include "imgui_impl_glfw.h"
#include "imgui_impl_vulkan.h"

#include "../../app/application.hpp"
#include "../../debug/macros.hpp"
#include "../../events/window.hpp"
#include "../../render/renderer.hpp"
#include "../../render/renderers/defines.hpp"
#include <vulkan/vulkan_core.h>
#include <vulkan/vulkan_enums.hpp>
#include <vulkan/vulkan_structs.hpp>

namespace Physbuzz {

ImGuiRenderer::ImGuiRenderer() {}

bool ImGuiRenderer::build() {
    std::vector<vk::DescriptorPoolSize> pool_sizes = {
        {vk::DescriptorType::eSampler, 1000},
        {vk::DescriptorType::eCombinedImageSampler, 1000},
        {vk::DescriptorType::eSampledImage, 1000},
        {vk::DescriptorType::eStorageImage, 1000},
        {vk::DescriptorType::eUniformTexelBuffer, 1000},
        {vk::DescriptorType::eStorageTexelBuffer, 1000},
        {vk::DescriptorType::eUniformBuffer, 1000},
        {vk::DescriptorType::eStorageBuffer, 1000},
        {vk::DescriptorType::eUniformBufferDynamic, 1000},
        {vk::DescriptorType::eStorageBufferDynamic, 1000},
        {vk::DescriptorType::eInputAttachment, 1000},
    };

    m_Pool = PBZ_VK_CHECK(App::Device.createDescriptorPool({
        .flags = vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet,
        .maxSets = 1000,
        .poolSizeCount = static_cast<std::uint32_t>(pool_sizes.size()),
        .pPoolSizes = pool_sizes.data(),
    }));

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    std::shared_ptr<Window> window = m_Scene->getSystem<Renderer>()->getInfo().window;
    m_Resolution = window->getResolution();

    if (!ImGui_ImplGlfw_InitForVulkan(*window, true)) {
        return false;
    }

    ImGui_ImplVulkan_InitInfo initInfo = {
        .Instance = App::Instance,
        .PhysicalDevice = App::PhysicalDevice,
        .Device = App::Device,
        .QueueFamily = App::Indices.graphics,
        .Queue = App::Queues.graphics,
        .PipelineCache = {},
        .DescriptorPool = m_Pool,
        .Subpass = {},
        .MinImageCount = detail::MAX_FRAMES_IN_FLIGHT,
        .ImageCount = detail::MAX_FRAMES_IN_FLIGHT,
        .MSAASamples = static_cast<VkSampleCountFlagBits>(vk::SampleCountFlagBits::e1),
        .UseDynamicRendering = true,
        .ColorAttachmentFormat = static_cast<VkFormat>(window->getInfo().swapChain.format),
        .Allocator = {},
        .CheckVkResultFn = [](VkResult err) {
            PBZ_VK_CHECK_RESULT(vk::Result(err));
        },
    };

    bool ret = ImGui_ImplVulkan_Init(&initInfo, VK_NULL_HANDLE);

    if (!ret) {
        return false;
    }

    m_Scene->getSystem<Renderer>()->immediate([](const vk::CommandBuffer &cmd) {
        ImGui_ImplVulkan_CreateFontsTexture(cmd);
    });

    ImGui_ImplVulkan_DestroyFontUploadObjects();

    m_Events = {
        .resize = window->addCallback<WindowSwapchainResizeEvent>([&](const auto &event) {
            resize(event.resolution);
        }),
    };

    return ret;
}

bool ImGuiRenderer::destroy() {
    m_Scene->getSystem<Renderer>()->getInfo().window->eraseCallback<WindowSwapchainResizeEvent>(m_Events.resize);

    App::Device.destroyDescriptorPool(m_Pool);
    m_Pool = nullptr;

    ImGui_ImplVulkan_Shutdown();
    ImGui_ImplGlfw_Shutdown();

    ImGui::DestroyContext();
    return true;
}

void ImGuiRenderer::newFrame() {
    ImGui_ImplVulkan_NewFrame();
    ImGui_ImplGlfw_NewFrame();
}

void ImGuiRenderer::render(const RenderContext &context) {
    {
        std::array barriers = {
            vk::ImageMemoryBarrier2{
                .srcStageMask = vk::PipelineStageFlagBits2::eTopOfPipe,
                .dstStageMask = vk::PipelineStageFlagBits2::eColorAttachmentOutput,
                .dstAccessMask = vk::AccessFlagBits2::eColorAttachmentWrite,
                .oldLayout = vk::ImageLayout::eUndefined,
                .newLayout = vk::ImageLayout::eColorAttachmentOptimal,
                .srcQueueFamilyIndex = vk::QueueFamilyIgnored,
                .dstQueueFamilyIndex = vk::QueueFamilyIgnored,
                .image = context.image,
                .subresourceRange = {
                    .aspectMask = vk::ImageAspectFlagBits::eColor,
                    .baseMipLevel = 0,
                    .levelCount = 1,
                    .baseArrayLayer = 0,
                    .layerCount = 1,
                },
            },
        };

        context.command.pipelineBarrier2({
            .dependencyFlags = {},
            .imageMemoryBarrierCount = barriers.size(),
            .pImageMemoryBarriers = barriers.data(),
        });
    }

    std::vector<vk::RenderingAttachmentInfo> colorAttachments = {
        {
            .imageView = context.imageView,
            .imageLayout = vk::ImageLayout::eColorAttachmentOptimal,
            .loadOp = vk::AttachmentLoadOp::eLoad,
            .storeOp = vk::AttachmentStoreOp::eStore,
        },
    };

    context.command.beginRendering({
        .renderArea = {
            .offset = {0, 0},
            .extent = {static_cast<std::uint32_t>(m_Resolution.x), static_cast<std::uint32_t>(m_Resolution.y)},
        },
        .layerCount = 1,
        .colorAttachmentCount = static_cast<std::uint32_t>(colorAttachments.size()),
        .pColorAttachments = colorAttachments.data(),
        .pDepthAttachment = {},
        .pStencilAttachment = {},
    });

    ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), context.command);

    context.command.endRendering();

    // transition image
    {
        vk::ImageMemoryBarrier2 barrier = {
            .srcStageMask = vk::PipelineStageFlagBits2::eColorAttachmentOutput,
            .srcAccessMask = vk::AccessFlagBits2::eColorAttachmentWrite,
            .dstStageMask = vk::PipelineStageFlagBits2::eBottomOfPipe,
            .oldLayout = vk::ImageLayout::eColorAttachmentOptimal,
            .newLayout = vk::ImageLayout::ePresentSrcKHR,
            .srcQueueFamilyIndex = vk::QueueFamilyIgnored,
            .dstQueueFamilyIndex = vk::QueueFamilyIgnored,
            .image = context.image,
            .subresourceRange = {
                .aspectMask = vk::ImageAspectFlagBits::eColor,
                .baseMipLevel = 0,
                .levelCount = 1,
                .baseArrayLayer = 0,
                .layerCount = 1,
            },
        };

        vk::DependencyInfo dependencyInfo = {
            .dependencyFlags = {},
            .imageMemoryBarrierCount = 1,
            .pImageMemoryBarriers = &barrier,
        };

        context.command.pipelineBarrier2(dependencyInfo);
    }
}

void ImGuiRenderer::resize(const glm::uvec2 &resolution) {
    m_Resolution = resolution;
}

} // namespace Physbuzz
