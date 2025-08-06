#pragma once

#include "../resources/resources.hpp"
#include <type_traits>
#include <vector>

namespace Physbuzz {

template <typename T>
concept VertexAttributeFormatType =
    std::is_class_v<T> &&
    std::is_trivial_v<T> &&
    std::is_standard_layout_v<T>;

enum class Types {
    Byte,
    UnsignedByte,
    Short,
    UnsignedShort,
    Int,
    UnsignedInt,

    Float,
    Double,
    HalfFloat,
    Fixed,
};

using Index = std::uint32_t;

class VertexAttribute {
  public:
    // let the user declare for now, reflection is ass and prone to breaking as of now.
    struct Format {
        Types type;
        std::int32_t size;
        std::int32_t offset;
    };

    struct Info {
        std::vector<Format> attributes;
        std::int32_t size;
    };

    VertexAttribute(const Info &info);

    bool build();
    bool destroy();

    const Info &getInfo() const;

  private:
    Info m_Info;

    friend class Mesh;
};

template <>
struct IsResource<VertexAttribute> : std::true_type {};

class Mesh {
  public:
    template <VertexAttributeFormatType T>
    struct Info {
        Resource<VertexAttribute> attribute;
        std::vector<T> vertices;
        std::vector<Index> indices;
    };

    template <VertexAttributeFormatType T>
    Mesh(const Info<T> &info)
        : m_Attribute(info.attribute),
          m_Indices(info.indices) {
        m_Vertices.resize(info.vertices.size() * sizeof(T));
        std::memcpy(m_Vertices.data(), info.vertices.data(), info.vertices.size() * sizeof(T));
    }

    bool build();
    bool destroy();

    void draw() const;

    const Resource<VertexAttribute> &getAttribute() const;
    const std::vector<std::byte> &getVertices() const;
    const std::vector<Index> &getIndices() const;

  private:
    Resource<VertexAttribute> m_Attribute;
    std::vector<std::byte> m_Vertices;
    std::vector<Index> m_Indices;
};

} // namespace Physbuzz
