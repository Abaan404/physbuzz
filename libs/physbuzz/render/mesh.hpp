#pragma once

#include "shaders.hpp"

namespace Physbuzz {

struct RenderComponent;

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

  private:
    Info m_Info;

    std::vector<vk::VertexInputAttributeDescription> m_Attributes;
    vk::VertexInputBindingDescription m_Binding;
    vk::PipelineVertexInputStateCreateInfo m_VertexInputStateCreateInfo;

    friend bool ShaderPipeline::build();
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
          m_VertexCount(info.vertices.size()),
          m_Indices(info.indices) {
        m_Vertices.resize(info.vertices.size() * sizeof(T));
        std::memcpy(m_Vertices.data(), info.vertices.data(), m_Vertices.size());
    }

    bool build();
    bool destroy();

    void draw(const vk::CommandBuffer &commandBuffer) const;

    const VertexDescription *getDescription() const;

  private:
    std::uint32_t findMemoryType(std::uint32_t typeFilter, vk::MemoryPropertyFlags properties);

    VertexDescription *m_Description = nullptr;
    std::uint32_t m_VertexCount = 0;

    struct {
        vk::Buffer buffer = nullptr;
        vk::DeviceMemory memory = nullptr;
    } m_Vertex = {};

    struct {
        vk::Buffer buffer = nullptr;
        vk::DeviceMemory memory = nullptr;
    } m_Index = {};

    std::vector<std::byte> m_Vertices = {};
    std::vector<Index> m_Indices = {};
};

} // namespace Physbuzz
