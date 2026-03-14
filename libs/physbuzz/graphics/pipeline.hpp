#pragma once

#include "../resources/resource.hpp"
#include <glm/glm.hpp>
#include <string>
#include <vulkan/vulkan.hpp>

namespace Physbuzz {

class PipelineLayout;
class VertexDescription;
struct RenderContext;

class RenderPipeline {
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
        std::string module;
        VertexDescription *description = nullptr;

        struct {
            bool primitiveRestartEnable = false;
            Topology topology = Topology::eTriangleList;
        } assembly = {};

        struct {
            bool depthClampEnable = false;
            bool rasterizerDiscardEnable = false;
            bool depthBiasEnable = false;
            float depthBiasSlopeFactor = 1.0f;
            float lineWidth = 1.0f;
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
            std::vector<Resource<PipelineLayout>> resources = {};
            std::vector<PushConstants> pushConstantRanges = {};
        } layouts = {};

        struct {
            std::vector<std::uint32_t> colors = {0};
            std::optional<std::uint32_t> depth = std::nullopt;
            std::optional<std::uint32_t> stencil = std::nullopt;
        } inputs = {};

        struct {
            std::vector<std::uint32_t> offsets;
            std::size_t size;
        } specialization;

        std::vector<DynamicState> dynamicStates = {};
    };

    RenderPipeline(const Info &info);

    bool build();
    bool destroy();

    bool reload(WatchAction action, const std::filesystem::path &path);

    template <typename T>
    bool specialize(const T &data) {
        std::span<const std::byte> bytes = std::as_bytes(std::span(&data, 1));
        return specialize(bytes);
    }

    bool specialize(const std::span<const std::byte> &data);

    void updatePushConstants(const RenderContext &context, const PushConstantsStage &stage, const std::span<const std::byte> &bytes, std::uint32_t offset);
    void bind(const RenderContext &context);

    const Info &getInfo() const;

  private:
    std::optional<vk::Pipeline> createSpecializedPipeline(const std::span<const std::byte> &specializationData);

    Info m_Info;

    vk::PipelineLayout m_Layout = nullptr;

    std::vector<vk::Pipeline> m_Pipelines;
    std::unordered_map<std::size_t, std::size_t> m_Specializations;
    std::size_t m_ActivePipeline = -1u;

    std::unordered_set<std::filesystem::path> m_DependencyFilePaths;

    friend class PipelineLayoutAllocator;
};

template <>
struct IsResource<RenderPipeline> : std::true_type {};

} // namespace Physbuzz
