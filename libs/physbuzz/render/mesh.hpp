#pragma once

#include "../resources/resources.hpp"
#include <glad/gl.h>
#include <glm/glm.hpp>
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
        std::vector<Format> formats;
        GLuint size;
    };

    VertexAttribute(const Info &info)
        : m_Info(info) {};

    bool build() {
        if (VAO != 0 && VBO != 0 && EBO != 0) {
            Logger::ERROR("[VertexAttribute] Cannot create already built vertex attributes.");
            return false;
        }

        glCreateBuffers(1, &VBO);
        glCreateBuffers(1, &EBO);
        glCreateVertexArrays(1, &VAO);

        std::size_t i = 0;
        for (const auto &format : m_Info.formats) {
            glEnableVertexArrayAttrib(VAO, i);
            glVertexArrayAttribBinding(VAO, i, 0);

            switch (format.type) {
            case Types::Byte:
            case Types::UnsignedByte:
            case Types::Short:
            case Types::UnsignedShort:
            case Types::Int:
            case Types::UnsignedInt:
                glVertexArrayAttribIFormat(VAO, i, format.size, static_cast<GLenum>(format.type), format.offset);
                break;

            case Types::HalfFloat:
            case Types::Fixed:
            case Types::Float:
                glVertexArrayAttribFormat(VAO, i, format.size, static_cast<GLenum>(format.type), GL_FALSE, format.offset);
                break;

            case Types::Double:
                glVertexArrayAttribLFormat(VAO, i, format.size, static_cast<GLenum>(format.type), format.offset);
                break;
            }
            i++;
        }

        glVertexArrayElementBuffer(VAO, EBO);
        glVertexArrayVertexBuffer(VAO, 0, VBO, 0, m_Info.size);

        return true;
    }

    bool destroy() {
        if (VAO == 0 && VBO == 0 && EBO == 0) {
            Logger::ERROR("[VertexAttribute] Cannot destroy already destructed vertex attributes.");
            return false;
        }

        glDeleteBuffers(1, &VBO);
        glDeleteBuffers(1, &EBO);
        glDeleteVertexArrays(1, &VAO);

        return true;
    }

    const Info &getInfo() const {
        return m_Info;
    }

  private:
    GLuint VBO = 0;
    GLuint VAO = 0;
    GLuint EBO = 0;

    Info m_Info;

    template <VertexAttributeFormatType>
    friend class Mesh;
};

template <>
struct IsResource<VertexAttribute> : std::true_type {};

template <VertexAttributeFormatType T>
class Mesh {
  public:
    struct Info {
        Resource<VertexAttribute> attribute;
        std::vector<T> vertices;
        std::vector<Index> indices;
    };

    Mesh(const Info &info)
        : m_Info(info) {}

    bool build() { return true; }
    bool destroy() { return true; }

    void draw() const {
        const VertexAttribute *attribute = m_Info.attribute.get();

        glNamedBufferData(attribute->VBO, m_Info.vertices.size() * sizeof(T), m_Info.vertices.data(), GL_STREAM_DRAW);
        glNamedBufferData(attribute->EBO, m_Info.indices.size() * sizeof(Index), m_Info.indices.data(), GL_STREAM_DRAW);
        glBindVertexArray(attribute->VAO);

        glDrawElements(GL_TRIANGLES, m_Info.indices.size(), GL_UNSIGNED_INT, 0);
    }

    const Info &getInfo() const {
        return m_Info;
    }

  private:
    Info m_Info;
};

} // namespace Physbuzz
