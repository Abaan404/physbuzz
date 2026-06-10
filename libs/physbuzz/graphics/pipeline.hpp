#pragma once

#include "../resources/resource.hpp"
#include "defines.hpp"
#include <glm/glm.hpp>
#include <string>
#include <vulkan/vulkan.hpp>

namespace Physbuzz {

class DescriptorLayout;
class VertexDescription;
struct RenderContext;

class Shader {
  public:
    struct Info {
        std::string module;

        struct {
            std::vector<std::uint32_t> offsets;
            std::size_t size;
        } specialization;
    };

    struct Data {
        std::vector<vk::PipelineShaderStageCreateInfo> stages;
        std::unordered_set<std::filesystem::path> dependencyFilePaths;
    };

    Shader(const Info &info);

    bool build(const std::span<const std::byte> &specializationData);
    bool destroy();

    const Info &getInfo() const;
    const Data &getData() const;

  private:
    Info m_Info;
    Data m_Data;

    std::vector<vk::SpecializationMapEntry> m_SpecializationEntries;
    vk::SpecializationInfo m_Specialization = {};
};

template <PipelineType T>
class Pipeline {
  public:
    using PushConstantsStage = vk::ShaderStageFlags;
    using PushConstantsStageFlags = vk::ShaderStageFlagBits;

    Pipeline(const Shader::Info &shaderInfo);

    bool build();
    bool destroy();

    bool reload(WatchAction action, const std::filesystem::path &path);

    void bind(const RenderContext &context);
    bool specialize(const std::span<const std::byte> &data);

    template <typename D>
    bool specialize(const D &data) {
        std::span<const std::byte> bytes = std::as_bytes(std::span(&data, 1));
        return specialize(bytes);
    }

    void updatePushConstants(const RenderContext &context, const PushConstantsStage &stage, const std::span<const std::byte> &bytes, std::uint32_t offset);

    vk::PipelineLayout getPipelineLayout() const;

  protected:
    std::unordered_set<std::filesystem::path> m_DependencyFilePaths;

  private:
    std::size_t calcSpecHash(const std::span<const std::byte> &data) const;

    Shader::Info m_ShaderInfo;

    vk::PipelineLayout m_Layout = nullptr;
    std::vector<vk::Pipeline> m_Pipelines;

    std::size_t m_ActivePipeline = -1u;
    std::unordered_map<std::size_t, std::size_t> m_Specializations;
};

class GraphicsPipeline : public Pipeline<GraphicsPipeline> {
  public:
    using DynamicState = vk::DynamicState;

    // assembly
    using Topology = vk::PrimitiveTopology;

    // multisample
    using SampleCountFlags = vk::SampleCountFlagBits;

    // color blend
    using ColorComponent = vk::ColorComponentFlags;
    using ColorComponentFlags = vk::ColorComponentFlagBits;
    using LogicOp = vk::LogicOp;
    using BlendOp = vk::BlendOp;
    using BlendFactor = vk::BlendFactor;

    // rasterization
    using CullMode = vk::CullModeFlags;
    using CullModeFlags = vk::CullModeFlagBits;
    using PolygonMode = vk::PolygonMode;
    using FrontFace = vk::FrontFace;

    // depthStencil
    using CompareOp = vk::CompareOp;

    // format
    using Format = vk::Format;

    // layouts
    using PushConstants = vk::PushConstantRange;
    using PushConstantsStage = vk::ShaderStageFlags;
    using PushConstantsStageFlags = vk::ShaderStageFlagBits;

    struct ColorBlendAttachmentInfo {
        bool blendEnable = false;
        BlendFactor srcColorBlendFactor = BlendFactor::eZero;
        BlendFactor dstColorBlendFactor = BlendFactor::eZero;
        BlendOp colorBlendOp = BlendOp::eAdd;
        BlendFactor srcAlphaBlendFactor = BlendFactor::eZero;
        BlendFactor dstAlphaBlendFactor = BlendFactor::eZero;
        BlendOp alphaBlendOp = BlendOp::eAdd;
        ColorComponent colorWriteMask = ColorComponentFlags::eR |
                                        ColorComponentFlags::eG |
                                        ColorComponentFlags::eB |
                                        ColorComponentFlags::eA;
    };

    struct Info {
        VertexDescription *description = nullptr;

        struct {
            bool primitiveRestartEnable = false;
            Topology topology = Topology::eTriangleList;
        } assembly = {};

        struct {
            bool depthClampEnable = false;
            bool rasterizerDiscardEnable = false;
            bool depthBiasEnable = false;
            float depthBiasConstantFactor = 0.0f;
            float depthBiasClamp = 0.0f;
            float depthBiasSlopeFactor = 0.0f;
            float lineWidth = 0.0f;
            PolygonMode polygonMode = PolygonMode::eFill;
            CullMode cullMode = CullModeFlags::eBack;
            FrontFace frontFace = FrontFace::eCounterClockwise;
        } rasterization = {};

        struct {
            bool sampleShadingEnable = false;
            SampleCountFlags rasterizationSamples = SampleCountFlags::e1;
        } multisample = {};

        struct {
            bool depthTestEnable = vk::True;
            bool depthWriteEnable = vk::True;
            CompareOp depthCompareOp = vk::CompareOp::eLessOrEqual;
            bool stencilTestEnable = vk::False;
        } depthStencil = {};

        struct {
            bool logicOpEnable = false;
            LogicOp logicOp = LogicOp::eCopy;
            std::vector<ColorBlendAttachmentInfo> attachments = {1, {{}}};
            std::array<float, 4> constants = {0.0f, 0.0f, 0.0f, 0.0f};
        } blend = {};

        struct {
            std::vector<Format> color = {Format::eR8G8B8A8Srgb};
            Format depth = Format::eD32SfloatS8Uint;
            std::uint32_t viewMask = 0;
        } formats = {};

        struct {
            std::vector<Resource<DescriptorLayout>> resources = {};
            std::vector<PushConstants> pushConstantRanges = {};
        } layouts = {};

        struct {
            std::vector<std::uint32_t> colors = {0};
            std::optional<std::uint32_t> depth = std::nullopt;
            std::optional<std::uint32_t> stencil = std::nullopt;
        } inputs = {};

        std::vector<DynamicState> dynamicStates = {};
    };

    GraphicsPipeline(const Shader::Info &shaderInfo, const Info &info);

    const Info &getInfo() const;

  private:
    Info m_Info;

    void bindImpl(const RenderContext &context, vk::Pipeline pipeline);

    std::optional<vk::PipelineLayout> createPipelineLayoutImpl();
    std::optional<vk::Pipeline> createPipelineImpl(const Shader::Info &shaderInfo, const std::span<const std::byte> &specializationData);

    friend class Pipeline<GraphicsPipeline>;
};

class ComputePipeline : public Pipeline<ComputePipeline> {
  public:
    // layouts
    using PushConstants = vk::PushConstantRange;
    using PushConstantsStage = vk::ShaderStageFlags;
    using PushConstantsStageFlags = vk::ShaderStageFlagBits;

    struct Info {
        struct {
            std::vector<Resource<DescriptorLayout>> resources = {};
            std::vector<PushConstants> pushConstantRanges = {};
        } layouts = {};
    };

    ComputePipeline(const Shader::Info &shaderInfo, const Info &info);

    const Info &getInfo() const;

  private:
    Info m_Info;

    void bindImpl(const RenderContext &context, vk::Pipeline pipeline);

    std::optional<vk::PipelineLayout> createPipelineLayoutImpl();
    std::optional<vk::Pipeline> createPipelineImpl(const Shader::Info &shaderInfo, const std::span<const std::byte> &specializationData);

    friend class Pipeline<ComputePipeline>;
};

template <>
struct IsResource<GraphicsPipeline> : std::true_type {};

template <>
struct IsResource<ComputePipeline> : std::true_type {};

} // namespace Physbuzz
