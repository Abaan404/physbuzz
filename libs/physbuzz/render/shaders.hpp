#pragma once

#include "../resources/defines.hpp"
#include "../resources/resources.hpp"
#include <glm/glm.hpp>
#include <slang-com-ptr.h>
#include <slang.h>
#include <string>
#include <vulkan/vulkan.hpp>

namespace Physbuzz {

class PipelineLayout;
class VertexDescription;
class RenderContext;

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

        struct {
            std::vector<Format> color = {Format::eB8G8R8A8Srgb};
            Format depth = Format::eD32Sfloat;
        } formats = {};

        std::string module;
        VertexDescription *description;
        std::vector<Resource<PipelineLayout>> layouts = {};

        std::vector<DynamicState> dynamicStates = {};
    };

    RenderPipeline(const Info &info);

    bool build();
    bool destroy();

    void bind(const RenderContext &context);

    const Info &getInfo() const;

  private:
    Info m_Info;

    vk::PipelineLayout m_Layout = nullptr;
    vk::Pipeline m_Pipeline = nullptr;

    struct {
        EventID reload;
    } m_Events;

    friend class PipelineLayoutAllocator;
};

template <>
struct IsResource<RenderPipeline> : std::true_type {};

} // namespace Physbuzz
