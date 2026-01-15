#pragma once

#include "../resources/defines.hpp"
#include "memory.hpp"

namespace Physbuzz {

struct RenderComponent;
struct RenderContext;

template <typename T>
concept VertexAttributeFormatType =
    std::is_class_v<T> &&
    std::is_trivial_v<T> &&
    std::is_standard_layout_v<T>;

using Index = std::uint32_t;

class VertexDescription {
  public:
    using Format = vk::Format;
    using InputRate = vk::VertexInputRate;

    struct Attribute {
        Format format;
        std::uint32_t size;
        std::uint32_t offset;
    };

    struct Info {
        std::vector<Attribute> attributes;
        std::uint32_t size;
        std::uint32_t binding;
        InputRate inputRate = InputRate::eVertex;
    };

    VertexDescription(const Info &info);

    const Info &getInfo() const;

  private:
    Info m_Info;

    std::vector<vk::VertexInputAttributeDescription> m_Attributes;
    vk::VertexInputBindingDescription m_Binding;
    vk::PipelineVertexInputStateCreateInfo m_VertexInputStateCreateInfo;

    friend class RenderPipeline;
    friend class Mesh;
};

template <typename T>
concept VertexDescriptionType =
    std::is_class_v<T> &&
    std::is_trivial_v<T> &&
    std::is_standard_layout_v<T> &&
    requires {
        { T::Description } -> std::same_as<VertexDescription &>;
    };

class Mesh {
  public:
    template <VertexDescriptionType T>
    struct Info {
        std::vector<T> vertices;
        std::vector<Index> indices;
    };

    template <VertexDescriptionType T>
    Mesh(const Info<T> &info)
        : m_Description(&T::Description),
          m_Vertex({
              .usage = Buffer::BufferUsageFlagBits::eVertexBuffer | Buffer::BufferUsageFlagBits::eTransferDst,
              .memoryUsage = Buffer::MemoryUsage::CPUToGPU,
          }),
          m_Index({
              .usage = Buffer::BufferUsageFlagBits::eIndexBuffer | Buffer::BufferUsageFlagBits::eTransferDst,
              .memoryUsage = Buffer::MemoryUsage::CPUToGPU,
          }),
          m_Indices(info.indices) {
        std::span<const std::byte> bytes = std::as_bytes(std::span(info.vertices));
        m_Vertices.assign(bytes.begin(), bytes.end());
    }

    bool build(const std::shared_ptr<Transfer> transfer);
    bool destroy();

    void draw(const RenderContext &context, std::uint32_t instances, std::uint32_t object) const;

    const VertexDescription *getDescription() const;

  private:
    VertexDescription *m_Description = nullptr;

    Buffer m_Vertex;
    Buffer m_Index;

    std::vector<std::byte> m_Vertices = {};
    std::vector<Index> m_Indices = {};
};

template <>
struct IsResource<Mesh> : std::true_type {};

} // namespace Physbuzz
