#include "imgui_impl_physbuzz.hpp"

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_vulkan.h"

#include "../../app/application.hpp"
#include "../../debug/macros.hpp"
#include "../../ecs/scene.hpp"
#include "../../events/descriptor.hpp"
#include "../../graphics/descriptors/sampler.hpp"
#include "../../graphics/descriptors/texture.hpp"
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

    m_Scene->getSystem<Renderer>()->immediate([](vk::CommandBuffer cmd) {
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
    // FIXME
    for (const auto &[resource, tuple] : m_Attachments) {
        const auto &[attachments, event] = tuple;

        for (const auto attachment : attachments) {
            ImGui_ImplVulkan_RemoveTexture(static_cast<VkDescriptorSet>(attachment));
        }

        resource->eraseCallback<OnAttachmentRebuild>(event);
    }

    // FIXME
    for (const auto &[resource, tuple] : m_Textures) {
        const auto &[texture, event] = tuple;

        ImGui_ImplVulkan_RemoveTexture(static_cast<VkDescriptorSet>(texture));
        resource->eraseCallback<OnAttachmentRebuild>(event);
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
        vk::ImageLayout imageLayout = vk::ImageLayout::eUndefined;

        switch (texture->getInfo().type) {
        case Texture::Type::Dim2D:
        case Texture::Type::Cube:
            imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
            break;
        }

        const Physbuzz::Texture::Data &textureData = texture->getData();

        vk::Sampler sampler = Builtin::RenderPipelineImGui::ResourceSampler->getData().sampler;
        if (texture->getInfo().sampler.getInfo().type != Sampler::Type::None) {
            sampler = texture->getInfo().sampler.getData().sampler;
        }

        ImTextureID textureId = ImGui_ImplVulkan_AddTexture(
            static_cast<VkSampler>(sampler),
            static_cast<VkImageView>(texture->getData().view),
            static_cast<VkImageLayout>(imageLayout));

        EventID reallocId = texture->addCallback<OnTextureRebuild>([this, texture](const OnTextureRebuild &event) {
            vk::ImageLayout imageLayout = vk::ImageLayout::eUndefined;

            switch (texture->getInfo().type) {
            case Texture::Type::Dim2D:
            case Texture::Type::Cube:
                imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
                break;
            }

            ImTextureID &textureId = std::get<0>(m_Textures.at(texture));
            event.context.deletionQueue->enqueue(textureId);

            vk::Sampler sampler = Builtin::RenderPipelineImGui::ResourceSampler->getData().sampler;
            if (event.texture->getInfo().sampler.getInfo().type != Sampler::Type::None) {
                sampler = event.texture->getInfo().sampler.getData().sampler;
            }

            textureId = ImGui_ImplVulkan_AddTexture(
                static_cast<VkSampler>(sampler),
                static_cast<VkImageView>(event.texture->getData().view),
                static_cast<VkImageLayout>(vk::ImageLayout::eShaderReadOnlyOptimal));
        });

        m_Textures[texture] = {textureId, reallocId};
    }

    return std::get<0>(m_Textures.at(texture));
}

ImTextureID ImGuiRenderer::getTexture(const Resource<Attachment> &attachment, std::uint32_t frameInFlight) {
    PBZ_ASSERT(frameInFlight < detail::MAX_FRAMES_IN_FLIGHT, "[ImGuiRenderer] Invalid frame in flight");

    if (!m_Attachments.contains(attachment)) {
        vk::ImageLayout imageLayout = vk::ImageLayout::eUndefined;

        switch (attachment->getInfo().usage) {
        case Attachment::Usage::Color:
            imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
            break;

        case Attachment::Usage::Depth:
            imageLayout = vk::ImageLayout::eDepthReadOnlyOptimal;
            break;

        case Attachment::Usage::Stencil:
            imageLayout = vk::ImageLayout::eStencilReadOnlyOptimal;
            break;

        case Attachment::Usage::DepthStencil:
            imageLayout = vk::ImageLayout::eDepthStencilReadOnlyOptimal;
            break;
        }

        if (attachment->getInfo().sampler.getInfo().type != Sampler::Type::None) {
            imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
        }

        const std::array<Physbuzz::Attachment::Data, detail::MAX_FRAMES_IN_FLIGHT> &attachmentData = attachment->getRingData();
        std::array<ImTextureID, detail::MAX_FRAMES_IN_FLIGHT> textureIds;

        for (std::size_t i = 0; i < detail::MAX_FRAMES_IN_FLIGHT; i++) {
            textureIds[i] = ImGui_ImplVulkan_AddTexture(
                static_cast<VkSampler>(Builtin::RenderPipelineImGui::ResourceSampler->getData().sampler),
                static_cast<VkImageView>(attachmentData[i].view),
                static_cast<VkImageLayout>(imageLayout));
        }

        EventID event = attachment->addCallback<OnAttachmentRebuild>([this, attachment](const OnAttachmentRebuild &event) {
            vk::ImageLayout imageLayout = vk::ImageLayout::eUndefined;

            switch (attachment->getInfo().usage) {
            case Attachment::Usage::Color:
                imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
                break;

            case Attachment::Usage::Depth:
                imageLayout = vk::ImageLayout::eDepthReadOnlyOptimal;
                break;

            case Attachment::Usage::Stencil:
                imageLayout = vk::ImageLayout::eStencilReadOnlyOptimal;
                break;

            case Attachment::Usage::DepthStencil:
                imageLayout = vk::ImageLayout::eDepthStencilReadOnlyOptimal;
                break;
            }

            if (attachment->getInfo().sampler.getInfo().type != Sampler::Type::None) {
                imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
            }

            ImTextureID &textureId = std::get<0>(m_Attachments.at(attachment))[event.context.frameInFlight];
            event.context.deletionQueue->enqueue(textureId);

            const Physbuzz::Attachment::Data &attachmentData = event.attachment->getRingData()[event.context.frameInFlight];
            textureId = ImGui_ImplVulkan_AddTexture(
                static_cast<VkSampler>(Builtin::RenderPipelineImGui::ResourceSampler->getData().sampler),
                static_cast<VkImageView>(attachmentData.view),
                static_cast<VkImageLayout>(imageLayout));
        });

        m_Attachments[attachment] = {textureIds, event};

        return textureIds[frameInFlight];
    }

    return std::get<0>(m_Attachments.at(attachment))[frameInFlight];
}

} // namespace Physbuzz
