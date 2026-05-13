#include "imgui_impl_physbuzz.hpp"

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_vulkan.h"

#include "../../app/application.hpp"
#include "../../app/deletion.hpp"
#include "../../debug/macros.hpp"
#include "../../ecs/scene.hpp"
#include "../../events/descriptor.hpp"
#include "../../graphics/descriptors/sampler.hpp"
#include "../../graphics/descriptors/texture.hpp"

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

    ImGui_ImplVulkan_InitInfo initInfo = {
        .ApiVersion = vk::ApiVersion14,
        .Instance = App::Instance,
        .PhysicalDevice = App::PhysicalDevice,
        .Device = App::Device,
        .QueueFamily = App::Indices.graphics,
        .Queue = App::Queues.graphics,
        .DescriptorPool = nullptr, // let imgui create a pool
        .DescriptorPoolSize = 1000,
        .MinImageCount = detail::MAX_FRAMES_IN_FLIGHT,
        .ImageCount = detail::MAX_FRAMES_IN_FLIGHT,
        .PipelineInfoMain = {
            .RenderPass = {},
            .Subpass = {},
            .MSAASamples = static_cast<VkSampleCountFlagBits>(vk::SampleCountFlagBits::e1),
            .PipelineRenderingCreateInfo = vk::PipelineRenderingCreateInfo{
                .colorAttachmentCount = 1,
                .pColorAttachmentFormats = &m_Info.window->getInfo().swapChain.format,
            },
        },
        .UseDynamicRendering = true,
        .CheckVkResultFn = [](VkResult err) {
            PBZ_VK_CHECK_RESULT(vk::Result(err));
        },
    };

    if (!ImGui_ImplVulkan_Init(&initInfo)) {
        Logger::ERROR("[ImGuiRenderer] Could not initialize ImGui with Vulkan.");
        return false;
    }

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
            ImGui_ImplVulkan_RemoveTexture(reinterpret_cast<VkDescriptorSet>(attachment));
        }

        resource->eraseCallback<OnAttachmentRebuild>(event);
    }

    // FIXME
    for (const auto &[resource, tuple] : m_Textures) {
        const auto &[texture, event] = tuple;

        ImGui_ImplVulkan_RemoveTexture(reinterpret_cast<VkDescriptorSet>(texture));
        resource->eraseCallback<OnAttachmentRebuild>(event);
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

// TODO fixme

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

        // ImTextureID textureId = reinterpret_cast<ImTextureID>(ImGui_ImplVulkan_AddTexture(
        //     static_cast<VkSampler>(sampler),
        //     static_cast<VkImageView>(texture->getData().view),
        //     static_cast<VkImageLayout>(imageLayout)));

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

            // textureId = reinterpret_cast<ImTextureID>(ImGui_ImplVulkan_AddTexture(
            //     static_cast<VkSampler>(sampler),
            //     static_cast<VkImageView>(event.texture->getData().view),
            //     static_cast<VkImageLayout>(vk::ImageLayout::eShaderReadOnlyOptimal)));
        });

        // m_Textures[texture] = {textureId, reallocId};
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
            // textureIds[i] = reinterpret_cast<ImTextureID>(ImGui_ImplVulkan_AddTexture(
            //     static_cast<VkSampler>(Builtin::RenderPipelineImGui::ResourceSampler->getData().sampler),
            //     static_cast<VkImageView>(attachmentData[i].view),
            //     static_cast<VkImageLayout>(imageLayout)));
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
            // textureId = reinterpret_cast<ImTextureID>(ImGui_ImplVulkan_AddTexture(
            //     static_cast<VkSampler>(Builtin::RenderPipelineImGui::ResourceSampler->getData().sampler),
            //     static_cast<VkImageView>(attachmentData.view),
            //     static_cast<VkImageLayout>(imageLayout)));
        });

        m_Attachments[attachment] = {textureIds, event};

        return textureIds[frameInFlight];
    }

    return std::get<0>(m_Attachments.at(attachment))[frameInFlight];
}

} // namespace Physbuzz
