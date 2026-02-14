#include "imgui_impl_physbuzz.hpp"

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_vulkan.h"

#include "../../app/application.hpp"
#include "../../debug/macros.hpp"
#include "../../graphics/descriptors/sampler.hpp"
#include "../../graphics/renderer.hpp"

namespace Physbuzz {

namespace Builtin {

bool RenderPipelineImGui::build() {
    bool success = true;

    if (!ResourceRegistry<Sampler>::contains(ResourceSampler)) {
        success &= ResourceRegistry<Sampler>::insert(
            ResourceSampler,
            {{
                .type = Physbuzz::Sampler::Type::Linear,
            }});
    }

    return success;
}

} // namespace Builtin

ImGuiRenderer::ImGuiRenderer(const Info &info)
    : m_Info(info) {}

bool ImGuiRenderer::build() {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    if (!Builtin::RenderPipelineImGui::build()) {
        Logger::ERROR("[ImGuiRenderer] Could not build ImGui resources.");
        return false;
    }

    if (!ImGui_ImplGlfw_InitForVulkan(*m_Info.window, true)) {
        Logger::ERROR("[ImGuiRenderer] Could not initialize ImGui with GLFW.");
        return false;
    }

    if (!m_Pool) {
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
        .ColorAttachmentFormat = static_cast<VkFormat>(m_Info.window->getInfo().swapChain.format),
        .Allocator = {},
        .CheckVkResultFn = [](VkResult err) {
            PBZ_VK_CHECK_RESULT(vk::Result(err));
        },
    };

    if (!ImGui_ImplVulkan_Init(&initInfo, VK_NULL_HANDLE)) {
        Logger::ERROR("[ImGuiRenderer] Could not initialize ImGui with Vulkan.");
        return false;
    }

    m_Scene->getSystem<Renderer>()->immediate([](const vk::CommandBuffer &cmd) {
        ImGui_ImplVulkan_CreateFontsTexture(cmd);
    });

    ImGui_ImplVulkan_DestroyFontUploadObjects();

    m_Graph.add(
        Output,
        {
            .execute = [](Scene *, const RenderContext &context) {
                std::vector<vk::RenderingAttachmentInfo> colorAttachments = {
                    {
                        .imageView = context.color.view,
                        .imageLayout = vk::ImageLayout::eColorAttachmentOptimal,
                        .loadOp = vk::AttachmentLoadOp::eLoad,
                        .storeOp = vk::AttachmentStoreOp::eStore,
                    },
                };

                context.command.beginRendering({
                    .renderArea = {
                        .offset = {0, 0},
                        .extent = context.extent,
                    },
                    .layerCount = 1,
                    .colorAttachmentCount = static_cast<std::uint32_t>(colorAttachments.size()),
                    .pColorAttachments = colorAttachments.data(),
                    .pDepthAttachment = {},
                    .pStencilAttachment = {},
                });

                ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), context.command);

                context.command.endRendering();
            },
        });

    return m_Graph.compile();
}

bool ImGuiRenderer::destroy() {
    for (const auto &[texture, id] : m_Textures) {
        ImGui_ImplVulkan_RemoveTexture(static_cast<VkDescriptorSet>(id));
    }

    if (m_Pool) {
        App::Device.destroyDescriptorPool(m_Pool);
        m_Pool = nullptr;
    }

    ImGui_ImplVulkan_Shutdown();
    ImGui_ImplGlfw_Shutdown();

    ImGui::DestroyContext();
    return true;
}

void ImGuiRenderer::newFrame() {
    ImGui_ImplVulkan_NewFrame();
    ImGui_ImplGlfw_NewFrame();
}

const RenderGraph &ImGuiRenderer::getGraph() const {
    return m_Graph;
}

ImTextureID ImGuiRenderer::getTexture(const Resource<Texture> &texture) {
    if (!m_Textures.contains(texture)) {
        const Physbuzz::Texture::Data &textureData = texture->getData();

        vk::Sampler sampler = nullptr;

        if (texture->getInfo().sampler.type != Sampler::Type::None) {
            sampler = texture->getData().sampler.getData().sampler;
        } else {
            sampler = Builtin::RenderPipelineImGui::ResourceSampler->getData().sampler;
        }

        m_Textures.insert({
            texture,
            ImGui_ImplVulkan_AddTexture(
                static_cast<VkSampler>(sampler),
                static_cast<VkImageView>(texture->getData().view),
                static_cast<VkImageLayout>(texture->getData().layout)),
        });
    }

    return m_Textures.at(texture);
}

} // namespace Physbuzz
