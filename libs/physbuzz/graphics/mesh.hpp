#pragma once

#include "../resources/defines.hpp"
#include "memory.hpp"

namespace Physbuzz {

class TransferBatch;
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
        std::uint32_t binding = 0;
        InputRate inputRate = InputRate::eVertex;
    };

    VertexDescription(const Info &info);

    const Info &getInfo() const;

  private:
    Info m_Info;

    std::vector<vk::VertexInputAttributeDescription> m_Attributes;
    vk::VertexInputBindingDescription m_Binding;

    friend class RenderPipeline;
    friend class Mesh;
};

class Mesh {
  public:
    struct SubMesh {
        std::uint32_t indexCount;
        std::uint32_t firstIndex;
        std::uint32_t vertexOffset;
    };

    struct Info {
        const VertexDescription *description;
        std::uint64_t vertexCount;
        std::uint64_t indexCount;

        std::vector<SubMesh> submeshes;
    };

    Mesh(const Info &info)
        : m_Info(info) {}

    bool build();
    bool destroy();

    template <typename T>
    bool write(std::vector<T> &&vertices, std::vector<Index> &&indices, TransferBatch &batch) {
        std::vector<std::byte> vertexBytes(
            reinterpret_cast<std::byte *>(vertices.data()),
            reinterpret_cast<std::byte *>(vertices.data() + vertices.size()));

        std::vector<std::byte> indexBytes(
            reinterpret_cast<std::byte *>(indices.data()),
            reinterpret_cast<std::byte *>(indices.data() + indices.size()));

        return write(std::move(vertexBytes), std::move(indexBytes), batch);
    }

    bool write(std::vector<std::byte> &&vertices, std::vector<std::byte> &&indices, TransferBatch &batch);

    void draw(const RenderContext &context, std::uint32_t instanceCount, std::uint32_t meshOffset) const;

    const Info &getInfo() const;

  private:
    Info m_Info;

    Buffer m_Vertices = {{
        .usage = Buffer::UsageFlagBits::eVertexBuffer | Buffer::UsageFlagBits::eTransferDst,
        .memoryUsage = Buffer::MemoryUsage::CPUToGPU,
    }};

    Buffer m_Indices = {{
        .usage = Buffer::UsageFlagBits::eIndexBuffer | Buffer::UsageFlagBits::eTransferDst,
        .memoryUsage = Buffer::MemoryUsage::CPUToGPU,
    }};
};

template <>
struct IsResource<Mesh> : std::true_type {};

} // namespace Physbuzz
