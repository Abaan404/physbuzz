#pragma once

#include "../io/file.hpp"
#include "../resources/defines.hpp"
#include "../resources/resources.hpp"
#include <glm/glm.hpp>
#include <string>
#include <unordered_map>
#include <vulkan/vulkan.hpp>

namespace Physbuzz {

class PipelineLayout;
class VertexDescription;

class RenderPipeline {
  public:
    using ShaderStageFlags = vk::ShaderStageFlagBits;
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

    // layout
    using DescriptorType = vk::DescriptorType;

    // format
    using Format = vk::Format;

    struct ShaderInfo {
        File::Info module;
        std::string entrypoint;
    };

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

    struct Layout {
        DescriptorType type;
        ShaderStageFlags stage;
        std::uint32_t binding;
        std::uint32_t count;
    };

    struct Info {
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
            std::array<float, 4> blendConstants = {0.0f, 0.0f, 0.0f, 0.0f};
        } blend = {};

        Format format = Format::eB8G8R8A8Srgb;

        std::vector<Resource<PipelineLayout>> layouts = {};

        VertexDescription *description;

        std::unordered_map<ShaderStageFlags, ShaderInfo> shaders;
        std::vector<DynamicState> dynamicStates = {};
    };

    RenderPipeline(const Info &info);
    ~RenderPipeline();

    bool build();
    bool destroy();

    void bind(const vk::CommandBuffer &commandBuffer) const;

    const Info &getInfo() const;

  private:
    vk::PipelineLayout m_Layout = nullptr;
    vk::Pipeline m_Pipeline = nullptr;

    Info m_Info;

    bool buildShaders(const std::unordered_map<ShaderStageFlags, ShaderInfo> &shaders, std::unordered_map<ShaderStageFlags, vk::ShaderModule> &modules);
    bool destroyShaders(const std::unordered_map<ShaderStageFlags, vk::ShaderModule> &modules);

    // template <ResourceType T>
    // friend class ResourceRegistry;

    friend class PipelineLayoutAllocator;
};

template <>
struct IsResource<RenderPipeline> : std::true_type {};

} // namespace Physbuzz
