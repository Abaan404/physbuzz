#include "shaders.hpp"

#include "../app/application.hpp"
#include "../debug/macros.hpp"
#include "layout.hpp"
#include "mesh.hpp"
#include <filesystem>
#include <glm/gtc/type_ptr.hpp>
#include <vulkan/vulkan_enums.hpp>
#include <vulkan/vulkan_handles.hpp>

namespace Physbuzz {

RenderPipeline::RenderPipeline(const Info &info)
    : m_Info(info) {}

bool RenderPipeline::build() {
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

    vk::PipelineDepthStencilStateCreateInfo depthStencil = {
        .depthTestEnable = m_Info.depthStencil.depthTestEnable,
        .depthWriteEnable = m_Info.depthStencil.depthWriteEnable,
        .depthCompareOp = m_Info.depthStencil.depthCompareOp,
        .stencilTestEnable = m_Info.depthStencil.stencilTestEnable,
    };

    std::filesystem::path resourcePath = ResourceRegistry<RenderPipeline>::getResourceDirectory();

    std::vector<slang::CompilerOptionEntry> compilerOptions;

#if !defined(NDEBUG)
    compilerOptions.emplace_back<slang::CompilerOptionEntry>({
        .name = slang::CompilerOptionName::DebugInformation,
        .value = {slang::CompilerOptionValueKind::Int, SLANG_DEBUG_INFO_LEVEL_STANDARD},
    });
#endif

    std::array targets = {slang::TargetDesc{
        .format = SLANG_SPIRV,
        .profile = App::SlangSession->findProfile("spirv_1_3"), // Driver bug: OpCopyLogical seems to cause a crash when RADV tries to compile to nir
                                                                // SPIR-V 1.3 doesn't use OpCopyLogical, target this version.
                                                                // TODO investigate further
        .compilerOptionEntries = compilerOptions.data(),
        .compilerOptionEntryCount = static_cast<std::uint32_t>(compilerOptions.size()),
    }};
    std::array searchPaths = {resourcePath.c_str()};

    slang::SessionDesc sessionDesc = {
        .targets = targets.data(),
        .targetCount = targets.size(),
        .searchPaths = searchPaths.data(),
        .searchPathCount = searchPaths.size(),
    };

    Slang::ComPtr<slang::ISession> session = nullptr;
    App::SlangSession->createSession(sessionDesc, session.writeRef());

    Slang::ComPtr<slang::IBlob> diagnostics;
    Slang::ComPtr<slang::IModule> module = Slang::ComPtr{session->loadModule(m_Info.module.c_str(), diagnostics.writeRef())};

    if (diagnostics) {
        Logger::ERROR("[RenderPipeline] Failed loading shader module '{}'", m_Info.module);
        Logger::ERROR((const char *)diagnostics->getBufferPointer());
        return false;
    }

    for (int i = 0; i < module->getDependencyFileCount(); i++) {
        m_DependencyFilePaths.insert(module->getDependencyFilePath(i));
    }

    std::vector<slang::IComponentType *> components;
    components.push_back(module);

    for (int i = 0; i < module->getDefinedEntryPointCount(); i++) {
        Slang::ComPtr<slang::IEntryPoint> entrypoint;
        module->getDefinedEntryPoint(i, entrypoint.writeRef());
        components.push_back(entrypoint);
    }

    Slang::ComPtr<slang::IComponentType> program;
    session->createCompositeComponentType(components.data(), components.size(), program.writeRef(), diagnostics.writeRef());

    if (diagnostics) {
        Logger::ERROR("[RenderPipeline] Failed composing shader module '{}'", m_Info.module);
        Logger::ERROR((const char *)diagnostics->getBufferPointer());
        return false;
    }

    slang::ProgramLayout *layout = program->getLayout(0, diagnostics.writeRef());

    if (diagnostics) {
        Logger::ERROR("[RenderPipeline] Failed loading shader layout for module '{}'", m_Info.module);
        Logger::ERROR((const char *)diagnostics->getBufferPointer());
        return false;
    }

    Slang::ComPtr<slang::IComponentType> linkedProgram;
    program->link(linkedProgram.writeRef(), diagnostics.writeRef());

    if (diagnostics) {
        Logger::ERROR("[RenderPipeline] Failed linking shader module '{}'", m_Info.module);
        Logger::ERROR((const char *)diagnostics->getBufferPointer());
        return false;
    }

    std::vector<vk::ShaderModule> modules;
    std::vector<vk::PipelineShaderStageCreateInfo> stages;

    for (std::size_t i = 0; i < layout->getEntryPointCount(); i++) {
        Slang::ComPtr<slang::IBlob> kernel;
        linkedProgram->getEntryPointCode(i, 0, kernel.writeRef(), diagnostics.writeRef());

        if (diagnostics) {
            Logger::ERROR("[RenderPipeline] Failed to get entrypoint '{}' code for module '{}'", i, m_Info.module);
            Logger::ERROR((const char *)diagnostics->getBufferPointer());
            return false;
        }

        slang::EntryPointReflection *entrypointLayout = layout->getEntryPointByIndex(i);

        vk::ShaderStageFlagBits stage;
        switch (entrypointLayout->getStage()) {
        case SLANG_STAGE_ANY_HIT:
            stage = vk::ShaderStageFlagBits::eAnyHitKHR;
            break;
        case SLANG_STAGE_CALLABLE:
            stage = vk::ShaderStageFlagBits::eCallableKHR;
            break;
        case SLANG_STAGE_CLOSEST_HIT:
            stage = vk::ShaderStageFlagBits::eClosestHitKHR;
            break;
        case SLANG_STAGE_COMPUTE:
            stage = vk::ShaderStageFlagBits::eCompute;
            break;
        case SLANG_STAGE_DOMAIN:
            stage = vk::ShaderStageFlagBits::eTessellationControl;
            break;
        case SLANG_STAGE_FRAGMENT:
            stage = vk::ShaderStageFlagBits::eFragment;
            break;
        case SLANG_STAGE_GEOMETRY:
            stage = vk::ShaderStageFlagBits::eGeometry;
            break;
        case SLANG_STAGE_HULL:
            stage = vk::ShaderStageFlagBits::eTessellationEvaluation;
            break;
        case SLANG_STAGE_INTERSECTION:
            stage = vk::ShaderStageFlagBits::eIntersectionKHR;
            break;
        case SLANG_STAGE_MISS:
            stage = vk::ShaderStageFlagBits::eMissKHR;
            break;
        case SLANG_STAGE_RAY_GENERATION:
            stage = vk::ShaderStageFlagBits::eRaygenKHR;
            break;
        case SLANG_STAGE_VERTEX:
            stage = vk::ShaderStageFlagBits::eVertex;
            break;
        case SLANG_STAGE_MESH:
            stage = vk::ShaderStageFlagBits::eMeshEXT;
            break;
        case SLANG_STAGE_AMPLIFICATION:
            stage = vk::ShaderStageFlagBits::eTaskEXT;
            break;
        default:
            continue;
        }

        vk::ShaderModule module = modules.emplace_back(PBZ_VK_CHECK(App::Device.createShaderModule({
            .codeSize = kernel->getBufferSize(),
            .pCode = reinterpret_cast<const std::uint32_t *>(kernel->getBufferPointer()),
        })));

        stages.emplace_back<vk::PipelineShaderStageCreateInfo>({
            .stage = stage,
            .module = module,
            .pName = "main",
            .pSpecializationInfo = nullptr, // TODO
        });
    }

    std::vector<vk::DescriptorSetLayout> layouts;
    for (const auto &layout : m_Info.layouts.resources) {
        layouts.emplace_back(layout->m_Layout);
    }

    m_Layout = PBZ_VK_CHECK(App::Device.createPipelineLayout({
        .setLayoutCount = static_cast<std::uint32_t>(layouts.size()),
        .pSetLayouts = layouts.data(),
        .pushConstantRangeCount = static_cast<std::uint32_t>(m_Info.layouts.pushConstantRanges.size()),
        .pPushConstantRanges = m_Info.layouts.pushConstantRanges.data(),
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
            .pDepthStencilState = &depthStencil,
            .pColorBlendState = &colorBlending,
            .pDynamicState = &dynamicState,
            .layout = m_Layout,
            .renderPass = nullptr,
        },
        {
            .colorAttachmentCount = static_cast<std::uint32_t>(m_Info.formats.color.size()),
            .pColorAttachmentFormats = m_Info.formats.color.data(),
            .depthAttachmentFormat = m_Info.formats.depth,
        },
    };

    m_Pipeline = PBZ_VK_CHECK(App::Device.createGraphicsPipeline(nullptr, chain.get()));

    for (const auto &module : modules) {
        App::Device.destroyShaderModule(module);
    }

    return true;
}

bool RenderPipeline::destroy() {
    if (m_Pipeline == nullptr) {
        Logger::WARNING("[ShaderPipeline] Trying to destroy a destructed pipeline.");
        return true;
    }

    App::Device.destroyPipelineLayout(m_Layout);
    m_Layout = nullptr;

    App::Device.destroyPipeline(m_Pipeline);
    m_Pipeline = nullptr;
    return true;
}

bool RenderPipeline::isDependantFile(const std::filesystem::path &file) {
    return m_DependencyFilePaths.contains(std::filesystem::weakly_canonical(file));
}

void RenderPipeline::updatePushConstants(const RenderContext &context, const PushConstantsStage &stage, const std::span<const std::byte> &bytes, std::uint32_t offset) {
    vk::PushConstantsInfo info = {
        .layout = m_Layout,
        .stageFlags = stage,
        .offset = offset,
        .size = static_cast<std::uint32_t>(bytes.size()),
        .pValues = bytes.data(),
    };

    context.command.pushConstants2(info);
}

void RenderPipeline::bind(const RenderContext &context) {
    context.command.bindPipeline(vk::PipelineBindPoint::eGraphics, m_Pipeline);
}

const RenderPipeline::Info &RenderPipeline::getInfo() const {
    return m_Info;
}

} // namespace Physbuzz
