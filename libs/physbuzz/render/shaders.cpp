#include "shaders.hpp"

#include "../app/application.hpp"
#include "../debug/macros.hpp"
#include "layout.hpp"
#include "mesh.hpp"
#include <glm/gtc/type_ptr.hpp>

namespace Physbuzz {

RenderPipeline::RenderPipeline(const Info &info)
    : m_Info(info) {}

RenderPipeline::~RenderPipeline() {}

bool RenderPipeline::build() {
    PBZ_ASSERT(App::Device != nullptr, "[RenderPipeline] App::build() not called.");

    if (m_Pipeline != nullptr) {
        Logger::WARNING("[RenderPipeline] Trying to build a constructed pipeline.");
        return true;
    }

    if (m_Info.blend.attachments.size() < 1) {
        Logger::ERROR("[RenderPipeline] No blend attachments attached.");
        return false;
    }

    if (std::ranges::none_of(m_Info.dynamicStates, [](vk::DynamicState state) {
            return state == vk::DynamicState::eViewport;
        })) {
        m_Info.dynamicStates.push_back(vk::DynamicState::eViewport);
    }

    if (std::ranges::none_of(m_Info.dynamicStates, [](vk::DynamicState state) {
            return state == vk::DynamicState::eScissor;
        })) {
        m_Info.dynamicStates.push_back(vk::DynamicState::eScissor);
    }

    vk::PipelineDynamicStateCreateInfo dynamicState = {
        .dynamicStateCount = static_cast<std::uint32_t>(m_Info.dynamicStates.size()),
        .pDynamicStates = m_Info.dynamicStates.data(),
    };

    vk::PipelineInputAssemblyStateCreateInfo inputAssembly = {
        .topology = m_Info.assembly.topology,
        .primitiveRestartEnable = m_Info.assembly.primitiveRestartEnable ? vk::True : vk::False,
    };

    vk::PipelineViewportStateCreateInfo viewportState = {
        .viewportCount = 1,
        .scissorCount = 1,
    };

    vk::PipelineRasterizationStateCreateInfo rasterizer = {
        .depthClampEnable = m_Info.rasterization.depthClampEnable ? vk::True : vk::False,
        .rasterizerDiscardEnable = m_Info.rasterization.rasterizerDiscardEnable ? vk::True : vk::False,
        .polygonMode = m_Info.rasterization.polygonMode,
        .cullMode = m_Info.rasterization.cullMode,
        .frontFace = m_Info.rasterization.frontFace,
        .depthBiasEnable = m_Info.rasterization.depthBiasEnable ? vk::True : vk::False,
        .depthBiasSlopeFactor = m_Info.rasterization.depthBiasSlopeFactor,
        .lineWidth = m_Info.rasterization.lineWidth,
    };

    std::vector<vk::PipelineColorBlendAttachmentState> colorBlendAttachments;
    for (const auto &attachment : m_Info.blend.attachments) {
        colorBlendAttachments.emplace_back<vk::PipelineColorBlendAttachmentState>({
            .blendEnable = attachment.blendEnable ? vk::True : vk::False,
            .srcColorBlendFactor = attachment.srcColorBlendFactor,
            .dstColorBlendFactor = attachment.dstColorBlendFactor,
            .colorBlendOp = attachment.colorBlendOp,
            .srcAlphaBlendFactor = attachment.srcAlphaBlendFactor,
            .dstAlphaBlendFactor = attachment.dstAlphaBlendFactor,
            .alphaBlendOp = attachment.alphaBlendOp,
            .colorWriteMask = attachment.colorWriteMask,
        });
    }

    vk::PipelineColorBlendStateCreateInfo colorBlending = {
        .logicOpEnable = m_Info.blend.logicOpEnable ? vk::True : vk::False,
        .logicOp = m_Info.blend.logicOp,
        .attachmentCount = static_cast<std::uint32_t>(colorBlendAttachments.size()),
        .pAttachments = colorBlendAttachments.data(),
        .blendConstants = m_Info.blend.blendConstants,
    };

    vk::PipelineMultisampleStateCreateInfo multisampling = {
        .rasterizationSamples = m_Info.multisample.rasterizationSamples,
        .sampleShadingEnable = m_Info.multisample.sampleShadingEnable ? vk::True : vk::False,
    };

    std::unordered_map<ShaderStageFlags, vk::ShaderModule> modules;
    if (!buildShaders(m_Info.shaders, modules)) {
        Logger::ERROR("[RenderPipeline] Failed to build pipeline shaders.");
        destroyShaders(modules);
        return false;
    }

    std::vector<vk::PipelineShaderStageCreateInfo> stages;
    for (const auto &[stage, module] : modules) {
        stages.emplace_back<vk::PipelineShaderStageCreateInfo>({
            .stage = stage,
            .module = module,
            .pName = m_Info.shaders[stage].entrypoint.c_str(),
            .pSpecializationInfo = nullptr, // TODO
        });
    }

    std::vector<vk::DescriptorSetLayout> layouts;
    for (const auto &layout : m_Info.layouts) {
        layouts.emplace_back(layout->m_Layout);
    }

    m_Layout = PBZ_VK_CHECK(App::Device.createPipelineLayout({
        .setLayoutCount = static_cast<std::uint32_t>(layouts.size()),
        .pSetLayouts = layouts.data(),
        .pushConstantRangeCount = 0,
    }));

    vk::StructureChain<vk::GraphicsPipelineCreateInfo, vk::PipelineRenderingCreateInfo> chain = {
        {
            .stageCount = static_cast<std::uint32_t>(stages.size()),
            .pStages = stages.data(),
            .pVertexInputState = &m_Info.description->m_VertexInputStateCreateInfo,
            .pInputAssemblyState = &inputAssembly,
            .pViewportState = &viewportState,
            .pRasterizationState = &rasterizer,
            .pMultisampleState = &multisampling,
            .pColorBlendState = &colorBlending,
            .pDynamicState = &dynamicState,
            .layout = m_Layout,
            .renderPass = nullptr,
        },
        {
            .colorAttachmentCount = 1,
            .pColorAttachmentFormats = &m_Info.format,
        },
    };

    m_Pipeline = PBZ_VK_CHECK(App::Device.createGraphicsPipeline(nullptr, chain.get()));
    destroyShaders(modules);

    return true;
}

bool RenderPipeline::destroy() {
    if (m_Pipeline == nullptr) {
        Logger::WARNING("[ShaderPipeline] Trying to destroy a destructed pipeline.");
        return true;
    }

    App::Device.destroyPipelineLayout(m_Layout);
    App::Device.destroyPipeline(m_Pipeline);
    return true;
}

void RenderPipeline::bind(const vk::CommandBuffer &commandBuffer) const {
    commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, m_Pipeline);
}

const RenderPipeline::Info &RenderPipeline::getInfo() const {
    return m_Info;
}

bool RenderPipeline::buildShaders(const std::unordered_map<ShaderStageFlags, ShaderInfo> &shaders, std::unordered_map<ShaderStageFlags, vk::ShaderModule> &modules) {
    bool success = true;

    for (const auto &[stage, shader] : shaders) {
        if (shader.module.path.empty()) {
            Logger::ERROR("[RenderPipeline] Missing module path for stage {}", vk::to_string(stage));
            success = false;
            break;
        }

        File file = {shader.module};

        if (!file.build()) {
            Logger::ERROR("[RenderPipeline] Could not build file '{}'", shader.module.path.string());
            success = false;
            continue;
        }

        if (!file.read()) {
            Logger::ERROR("[RenderPipeline] Could not read file '{}'", shader.module.path.string());
            file.destroy();
            success = false;
            continue;
        }

        const std::vector<std::byte> &buffer = file.getData().buffer;

        vk::ShaderModuleCreateInfo createInfo = {
            .codeSize = buffer.size(),
            .pCode = reinterpret_cast<const std::uint32_t *>(buffer.data()),
        };

        modules[stage] = PBZ_VK_CHECK(App::Device.createShaderModule(createInfo));

        if (!file.destroy()) {
            Logger::ERROR("[RenderPipeline] Could not destroy file '{}'", shader.module.path.string());
            success = false;
            continue;
        }
    }

    return success;
}

bool RenderPipeline::destroyShaders(const std::unordered_map<ShaderStageFlags, vk::ShaderModule> &modules) {
    for (auto &[_, module] : modules) {
        App::Device.destroyShaderModule(module);
    }

    return true;
}

} // namespace Physbuzz
