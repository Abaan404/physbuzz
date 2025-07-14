#pragma once

#include "../resources/resources.hpp"
#include <glad/gl.h>
#include <type_traits>
#include <vector>

namespace Physbuzz {

template <typename T>
concept VertexAttributeFormatType =
    std::is_class_v<T> &&
    std::is_trivial_v<T> &&
    std::is_standard_layout_v<T>;

enum class Types {
    Byte = GL_BYTE,
    UnsignedByte = GL_UNSIGNED_BYTE,
    Short = GL_SHORT,
    UnsignedShort = GL_UNSIGNED_SHORT,
    Int = GL_INT,
    UnsignedInt = GL_UNSIGNED_INT,

    Float = GL_FLOAT,
    Double = GL_DOUBLE,
    HalfFloat = GL_HALF_FLOAT,
    Fixed = GL_FIXED,
};

using Index = std::uint32_t;

class VertexAttribute {
  public:
    // let the user declare for now, reflection is ass and prone to breaking as of now.
    struct Format {
        Types type;
        GLuint size;
        GLuint offset;
    };

    struct Info {
        std::vector<Format> attributes;
        GLuint size;
    };

    VertexAttribute(const Info &info);

    bool build();
    bool destroy();

    const Info &getInfo() const;

  private:
    GLuint VBO = 0;
    GLuint VAO = 0;
    GLuint EBO = 0;

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
