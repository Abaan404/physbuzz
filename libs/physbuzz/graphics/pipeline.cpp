#include "pipeline.hpp"

#include "../app/application.hpp"
#include "../app/deletion.hpp"
#include "layout.hpp"
#include "mesh.hpp"

namespace Physbuzz {

Shader::Shader(const Info &info)
    : m_Info(info) {}

bool Shader::build(const std::span<const std::byte> &specializationData) {
    if (!m_Data.stages.empty()) {
        Logger::WARNING("[Shader] Trying to build a constructed shader.");
        return true;
    }

    if (!specializationData.empty()) {
        m_SpecializationEntries.reserve(m_Info.specialization.offsets.size());

        for (std::uint32_t i = 0; i < m_Info.specialization.offsets.size(); i++) {
            std::uint32_t nextOffset = m_Info.specialization.size;

            if (i + 1 < m_Info.specialization.offsets.size()) {
                nextOffset = m_Info.specialization.offsets[i + 1];
            }

            m_SpecializationEntries.emplace_back<vk::SpecializationMapEntry>({
                .constantID = i,
                .offset = m_Info.specialization.offsets[i],
                .size = nextOffset - m_Info.specialization.offsets[i],
            });
        }

        m_Specialization = {
            .mapEntryCount = static_cast<std::uint32_t>(m_SpecializationEntries.size()),
            .pMapEntries = m_SpecializationEntries.data(),
            .dataSize = specializationData.size(),
            .pData = specializationData.data(),
        };
    }

    std::vector<slang::CompilerOptionEntry> compilerOptions;

#if !defined(NDEBUG)
    compilerOptions.emplace_back<slang::CompilerOptionEntry>({
        .name = slang::CompilerOptionName::DebugInformation,
        .value = {slang::CompilerOptionValueKind::Int, SLANG_DEBUG_INFO_LEVEL_MAXIMAL},
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

    std::filesystem::path resourcePath = ResourceRegistry<GraphicsPipeline>::getResourceDirectory();
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
        destroy();
        return false;
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
        destroy();
        return false;
    }

    slang::ProgramLayout *layout = program->getLayout(0, diagnostics.writeRef());

    if (diagnostics) {
        Logger::ERROR("[RenderPipeline] Failed loading shader layout for module '{}'", m_Info.module);
        Logger::ERROR((const char *)diagnostics->getBufferPointer());
        destroy();
        return false;
    }

    Slang::ComPtr<slang::IComponentType> linkedProgram;
    program->link(linkedProgram.writeRef(), diagnostics.writeRef());

    if (diagnostics) {
        Logger::ERROR("[RenderPipeline] Failed linking shader module '{}'", m_Info.module);
        Logger::ERROR((const char *)diagnostics->getBufferPointer());
        destroy();
        return false;
    }

    m_Data.stages.reserve(layout->getEntryPointCount());

    for (std::size_t i = 0; i < layout->getEntryPointCount(); i++) {
        Slang::ComPtr<slang::IBlob> kernel;
        linkedProgram->getEntryPointCode(i, 0, kernel.writeRef(), diagnostics.writeRef());

        if (diagnostics) {
            Logger::ERROR("[RenderPipeline] Failed to get entrypoint '{}' code for module '{}'", i, m_Info.module);
            Logger::ERROR((const char *)diagnostics->getBufferPointer());
            destroy();
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

        vk::ShaderModule module = PBZ_VK_CHECK(App::Device.createShaderModule({
            .codeSize = kernel->getBufferSize(),
            .pCode = reinterpret_cast<const std::uint32_t *>(kernel->getBufferPointer()),
        }));

        m_Data.stages.emplace_back<vk::PipelineShaderStageCreateInfo>({
            .stage = stage,
            .module = module,
            .pName = "main",
            .pSpecializationInfo = m_Specialization.dataSize > 0 ? &m_Specialization : nullptr,
        });
    }

    m_Data.dependencyFilePaths.reserve(module->getDependencyFileCount());

    for (int i = 0; i < module->getDependencyFileCount(); i++) {
        m_Data.dependencyFilePaths.insert(module->getDependencyFilePath(i));
    }

    return true;
}

bool Shader::destroy() {
    if (m_Data.stages.empty()) {
        Logger::WARNING("[Shader] Trying to destroy a destructed shader.");
        return true;
    }

    for (const auto &stage : m_Data.stages) {
        App::Device.destroyShaderModule(stage.module);
    }

    m_Data = {};
    m_Specialization = {};
    m_SpecializationEntries = {};

    return true;
}

const Shader::Info &Shader::getInfo() const {
    return m_Info;
}

const Shader::Data &Shader::getData() const {
    return m_Data;
}

template <PipelineType T>
Pipeline<T>::Pipeline(const Shader::Info &shaderInfo)
    : m_ShaderInfo(shaderInfo) {}

template <PipelineType T>
bool Pipeline<T>::build() {
    if (!m_Pipelines.empty()) {
        Logger::WARNING("[Pipeline] Trying to build a constructed pipeline.");
        return true;
    }

    std::optional<vk::PipelineLayout> layout = static_cast<T *>(this)->createPipelineLayoutImpl();
    if (!layout) {
        destroy();
        return false;
    }

    m_Layout = *layout;

    // create a pipeline with default spec
    std::span<const std::byte> zero;
    if (!specialize(zero)) {
        destroy();
        return false;
    }

    return true;
}

template <PipelineType T>
bool Pipeline<T>::destroy() {
    if (m_Pipelines.empty()) {
        Logger::WARNING("[Pipeline] Trying to destroy a destructed pipeline.");
        return true;
    }

    App::Device.destroyPipelineLayout(m_Layout);
    m_Layout = nullptr;

    for (const auto &pipeline : m_Pipelines) {
        App::Device.destroyPipeline(pipeline);
    }

    m_Pipelines = {};
    m_Specializations = {};
    m_ActivePipeline = -1;

    return true;
}

template <PipelineType T>
bool Pipeline<T>::reload(WatchAction action, const std::filesystem::path &path) {
    if (action != WatchAction::Modified) {
        return false;
    }

    if (!m_DependencyFilePaths.contains(path)) {
        return false;
    }

    Logger::INFO("[Pipeline] Reloading resource '{}'.", m_ShaderInfo.module);

    T pipeline = {m_ShaderInfo, static_cast<T *>(this)->getInfo()};
    if (!pipeline.build()) {
        return false;
    }

    {
        std::lock_guard<std::mutex> lock(ResourceRegistry<T>::ReloadMutex);
        App::Deletion.enqueue(std::move(*this));

        *this = std::move(pipeline);
    }

    return true;
}

template <PipelineType T>
void Pipeline<T>::bind(const RenderContext &context) {
    static_cast<T *>(this)->bindImpl(context, m_Pipelines[m_ActivePipeline]);
}

template <PipelineType T>
bool Pipeline<T>::specialize(const std::span<const std::byte> &data) {
    std::size_t specHash = calcSpecHash(data);

    if (!m_Specializations.contains(specHash)) {
        std::optional<vk::Pipeline> pipeline = static_cast<T *>(this)->createPipelineImpl(m_ShaderInfo, data);

        if (!pipeline) {
            return false;
        }

        m_Pipelines.emplace_back(*pipeline);
        m_Specializations[specHash] = m_Pipelines.size() - 1;
    }

    m_ActivePipeline = m_Specializations.at(specHash);

    return true;
}

template <PipelineType T>
void Pipeline<T>::updatePushConstants(const RenderContext &context, const PushConstantsStage &stage, const std::span<const std::byte> &bytes, std::uint32_t offset) {
    vk::PushConstantsInfo info = {
        .layout = m_Layout,
        .stageFlags = stage,
        .offset = offset,
        .size = static_cast<std::uint32_t>(bytes.size()),
        .pValues = bytes.data(),
    };

    context.command.pushConstants2(info);
}

template <PipelineType T>
vk::PipelineLayout Pipeline<T>::getPipelineLayout() const {
    return m_Layout;
}

template <PipelineType T>
std::size_t Pipeline<T>::calcSpecHash(const std::span<const std::byte> &data) const {
    if (data.empty() || m_ShaderInfo.specialization.size != data.size()) {
        std::vector zero(m_ShaderInfo.specialization.size, std::byte(0));

        return std::hash<std::string_view>{}({
            reinterpret_cast<const char *>(zero.data()),
            zero.size(),
        });
    }

    return std::hash<std::string_view>{}({
        reinterpret_cast<const char *>(data.data()),
        data.size(),
    });
}

GraphicsPipeline::GraphicsPipeline(const Shader::Info &shaderInfo, const Info &info)
    : m_Info(info), Pipeline<GraphicsPipeline>(shaderInfo) {}

void GraphicsPipeline::bindImpl(const RenderContext &context, vk::Pipeline pipeline) {
    context.command.bindPipeline(vk::PipelineBindPoint::eGraphics, pipeline);
}

std::optional<vk::PipelineLayout> GraphicsPipeline::createPipelineLayoutImpl() {
    std::vector<vk::DescriptorSetLayout> layouts;
    for (const auto &layout : m_Info.layouts.resources) {
        layouts.emplace_back(layout->getData().layout);
    }

    return PBZ_VK_CHECK(App::Device.createPipelineLayout({
        .setLayoutCount = static_cast<std::uint32_t>(layouts.size()),
        .pSetLayouts = layouts.data(),
        .pushConstantRangeCount = static_cast<std::uint32_t>(m_Info.layouts.pushConstantRanges.size()),
        .pPushConstantRanges = m_Info.layouts.pushConstantRanges.data(),
    }));
}

std::optional<vk::Pipeline> GraphicsPipeline::createPipelineImpl(const Shader::Info &shaderInfo, const std::span<const std::byte> &specializationData) {
    Shader shader = shaderInfo;

    if (!shader.build(specializationData)) {
        return std::nullopt;
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
        .depthBiasConstantFactor = m_Info.rasterization.depthBiasConstantFactor,
        .depthBiasClamp = m_Info.rasterization.depthBiasClamp,
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
        .blendConstants = m_Info.blend.constants,
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

    vk::PipelineVertexInputStateCreateInfo vertexInputState = {
        .vertexBindingDescriptionCount = 0,
    };

    if (m_Info.description != nullptr) {
        vertexInputState = {
            .vertexBindingDescriptionCount = 1,
            .pVertexBindingDescriptions = &m_Info.description->getData().binding,
            .vertexAttributeDescriptionCount = static_cast<std::uint32_t>(m_Info.description->getData().attributes.size()),
            .pVertexAttributeDescriptions = m_Info.description->getData().attributes.data(),
        };
    }

    vk::StructureChain<vk::GraphicsPipelineCreateInfo, vk::PipelineRenderingCreateInfo, vk::RenderingInputAttachmentIndexInfo> chain = {
        {
            .stageCount = static_cast<std::uint32_t>(shader.getData().stages.size()),
            .pStages = shader.getData().stages.data(),
            .pVertexInputState = &vertexInputState,
            .pInputAssemblyState = &inputAssembly,
            .pViewportState = &viewportState,
            .pRasterizationState = &rasterizer,
            .pMultisampleState = &multisampling,
            .pDepthStencilState = &depthStencil,
            .pColorBlendState = &colorBlending,
            .pDynamicState = &dynamicState,
            .layout = getPipelineLayout(),
            .renderPass = nullptr,
        },
        {
            .viewMask = m_Info.formats.viewMask,
            .colorAttachmentCount = static_cast<std::uint32_t>(m_Info.formats.color.size()),
            .pColorAttachmentFormats = m_Info.formats.color.data(),
            .depthAttachmentFormat = m_Info.formats.depth,
        },
        {
            .colorAttachmentCount = static_cast<std::uint32_t>(m_Info.inputs.colors.size()),
            .pColorAttachmentInputIndices = m_Info.inputs.colors.data(),
            .pDepthInputAttachmentIndex = m_Info.inputs.depth.has_value() ? &(*m_Info.inputs.depth) : nullptr,
            .pStencilInputAttachmentIndex = m_Info.inputs.depth.has_value() ? &(*m_Info.inputs.depth) : nullptr,
        },
    };

    vk::Pipeline pipeline = PBZ_VK_CHECK(App::Device.createGraphicsPipeline(nullptr, chain.get()));

    m_DependencyFilePaths = shader.getData().dependencyFilePaths;
    shader.destroy();

    return pipeline;
}

const GraphicsPipeline::Info &GraphicsPipeline::getInfo() const {
    return m_Info;
}

ComputePipeline::ComputePipeline(const Shader::Info &shaderInfo, const Info &info)
    : m_Info(info), Pipeline<ComputePipeline>(shaderInfo) {}

const ComputePipeline::Info &ComputePipeline::getInfo() const {
    return m_Info;
}

void ComputePipeline::bindImpl(const RenderContext &context, vk::Pipeline pipeline) {
    context.command.bindPipeline(vk::PipelineBindPoint::eCompute, pipeline);
}

std::optional<vk::PipelineLayout> ComputePipeline::createPipelineLayoutImpl() {
    std::vector<vk::DescriptorSetLayout> layouts;
    for (const auto &layout : m_Info.layouts.resources) {
        layouts.emplace_back(layout->getData().layout);
    }

    return PBZ_VK_CHECK(App::Device.createPipelineLayout({
        .setLayoutCount = static_cast<std::uint32_t>(layouts.size()),
        .pSetLayouts = layouts.data(),
        .pushConstantRangeCount = static_cast<std::uint32_t>(m_Info.layouts.pushConstantRanges.size()),
        .pPushConstantRanges = m_Info.layouts.pushConstantRanges.data(),
    }));
}

std::optional<vk::Pipeline> ComputePipeline::createPipelineImpl(const Shader::Info &shaderInfo, const std::span<const std::byte> &specializationData) {
    Shader shader = shaderInfo;

    if (!shader.build(specializationData)) {
        return std::nullopt;
    }

    vk::StructureChain<vk::ComputePipelineCreateInfo> chain = {
        {
            .stage = {},
            .layout = getPipelineLayout(),
        },
    };

    for (const auto &shaderStage : shader.getData().stages) {
        if (shaderStage.stage == vk::ShaderStageFlagBits::eCompute) {
            chain.get<vk::ComputePipelineCreateInfo>().stage = shaderStage;
            break;
        }
    }

    vk::Pipeline pipeline = PBZ_VK_CHECK(App::Device.createComputePipeline(nullptr, chain.get()));

    m_DependencyFilePaths = shader.getData().dependencyFilePaths;
    shader.destroy();

    return pipeline;
}

template class Pipeline<GraphicsPipeline>;
template class Pipeline<ComputePipeline>;

} // namespace Physbuzz
