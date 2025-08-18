#include "shaders.hpp"

#include "../app/application.hpp"
#include "../debug/macros.hpp"
#include "mesh.hpp"
#include <glm/gtc/type_ptr.hpp>

namespace Physbuzz {

ShaderPipeline::ShaderPipeline(const Info &info)
    : m_Info(info) {}

ShaderPipeline::~ShaderPipeline() {}

bool ShaderPipeline::build() {
    PBZ_ASSERT(App::Device != nullptr, "[ShaderPipeline] App::build() not called.");

    if (m_Pipeline != nullptr) {
        Logger::WARNING("[ShaderPipeline] Trying to build a constructed pipeline.");
        return true;
    }

    if (m_Info.module.path.empty()) {
        return false;
    }

    File file = File(m_Info.module);
    if (!file.build()) {
        Logger::ERROR("[Shader] Could not build file '{}'", m_Info.module.path.string());
        return false;
    }

    if (!file.read()) {
        Logger::ERROR("[Shader] Could not read file '{}'", m_Info.module.path.string());
        file.destroy();
        return false;
    }

    const File::Data &data = file.getData();

    vk::ShaderModule shaderModule = PBZ_VK_CHECK(App::Device.createShaderModule({
        .codeSize = data.buffer.size(),
        .pCode = reinterpret_cast<const std::uint32_t *>(data.buffer.data()),
    }));

    std::vector<vk::PipelineShaderStageCreateInfo> stages;
    for (const auto &[stage, shader] : m_Info.shaders) {
        stages.emplace_back<vk::PipelineShaderStageCreateInfo>({
            .stage = stage,
            .module = shaderModule,
            .pName = shader.entrypoint.c_str(),
            .pSpecializationInfo = nullptr, // TODO
        });
    }

    vk::PipelineDynamicStateCreateInfo dynamicState = {
        .dynamicStateCount = static_cast<std::uint32_t>(m_Info.states.size()),
        .pDynamicStates = m_Info.states.data(),
    };

    vk::PipelineInputAssemblyStateCreateInfo inputAssembly = {
        .topology = m_Info.topology,
    };

    vk::PipelineViewportStateCreateInfo viewportState = {
        .viewportCount = 1,
        .scissorCount = 1,
    };

    vk::PipelineRasterizationStateCreateInfo rasterizer = {
        .depthClampEnable = vk::False,
        .rasterizerDiscardEnable = vk::False,
        .polygonMode = vk::PolygonMode::eFill,
        .cullMode = vk::CullModeFlagBits::eBack,
        .frontFace = vk::FrontFace::eClockwise,
        .depthBiasEnable = vk::False,
        .depthBiasSlopeFactor = 1.0f,
        .lineWidth = 1.0f,
    };

    vk::PipelineColorBlendAttachmentState colorBlendAttachment = {
        .blendEnable = vk::False,
        .colorWriteMask = vk::ColorComponentFlagBits::eR |
                          vk::ColorComponentFlagBits::eG |
                          vk::ColorComponentFlagBits::eB |
                          vk::ColorComponentFlagBits::eA,
    };

    vk::PipelineColorBlendStateCreateInfo colorBlending{
        .logicOpEnable = vk::False,
        .logicOp = vk::LogicOp::eCopy,
        .attachmentCount = 1,
        .pAttachments = &colorBlendAttachment,
    };

    vk::PipelineLayoutCreateInfo pipelineLayoutInfo = {
        .setLayoutCount = 0,
        .pushConstantRangeCount = 0,
    };

    m_Layout = PBZ_VK_CHECK(App::Device.createPipelineLayout(pipelineLayoutInfo));

    vk::PipelineMultisampleStateCreateInfo multisampling = {
        .rasterizationSamples = vk::SampleCountFlagBits::e1,
        .sampleShadingEnable = vk::False,
    };

    vk::Format colorFormat = vk::Format::eR8G8B8A8Snorm;
    vk::PipelineRenderingCreateInfo pipelineRenderingCreateInfo = {
        .colorAttachmentCount = 1,
        .pColorAttachmentFormats = &colorFormat,
    };

    vk::GraphicsPipelineCreateInfo pipelineInfo = {
        .pNext = &pipelineRenderingCreateInfo,
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
    };

    m_Pipeline = PBZ_VK_CHECK(App::Device.createGraphicsPipeline(nullptr, pipelineInfo));

    App::Device.destroyShaderModule(shaderModule);

    if (!file.destroy()) {
        Logger::ERROR("[Shader] Could not destroy file '{}'", m_Info.module.path.string());
        return false;
    }

    return true;
}

bool ShaderPipeline::destroy() {
    if (m_Pipeline == nullptr) {
        Logger::WARNING("[ShaderPipeline] Trying to destroy a destructed pipeline.");
        return true;
    }

    App::Device.destroyPipelineLayout(m_Layout);
    App::Device.destroyPipeline(m_Pipeline);
    return true;
}

bool ShaderPipeline::reload() {
    if (!m_RequestedReload) {
        // no reload was necessary, expected behaviour
        return true;
    }

    m_RequestedReload = false;

    if (!m_FailedReload && !destroy()) {
        Logger::ERROR("[ShaderPipeline] Reload failed.");
        return false;
    }

    if (!build()) {
        m_FailedReload = true;
        return false;
    }

    m_FailedReload = false;
    return true;
}

void ShaderPipeline::bind(const vk::CommandBuffer &commandBuffer) const {
    commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, m_Pipeline);
}

const ShaderPipeline::Info &ShaderPipeline::getInfo() const {
    return m_Info;
}

} // namespace Physbuzz
