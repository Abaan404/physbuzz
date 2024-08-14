#pragma once

#include "../resources/file.hpp"
#include <glad/gl.h>
#include <glm/glm.hpp>
#include <string>

namespace Physbuzz {

template <typename T>
concept UniformType =
    std::same_as<T, float> ||
    std::same_as<T, glm::vec2> ||
    std::same_as<T, glm::vec3> ||
    std::same_as<T, glm::vec4> ||
    std::same_as<T, int> ||
    std::same_as<T, glm::ivec2> ||
    std::same_as<T, glm::ivec3> ||
    std::same_as<T, glm::ivec4> ||
    std::same_as<T, unsigned int> ||
    std::same_as<T, glm::uvec2> ||
    std::same_as<T, glm::uvec3> ||
    std::same_as<T, glm::uvec4> ||
    std::same_as<T, glm::mat2> ||
    std::same_as<T, glm::mat3> ||
    std::same_as<T, glm::mat4> ||
    std::same_as<T, glm::mat2x3> ||
    std::same_as<T, glm::mat3x2> ||
    std::same_as<T, glm::mat2x4> ||
    std::same_as<T, glm::mat4x2> ||
    std::same_as<T, glm::mat3x4> ||
    std::same_as<T, glm::mat4x3>;

enum class ShaderType {
    Vertex = GL_VERTEX_SHADER,
    TessControl = GL_TESS_CONTROL_SHADER,
    TessEvaluation = GL_TESS_EVALUATION_SHADER,
    Geometry = GL_GEOMETRY_SHADER,
    Fragment = GL_FRAGMENT_SHADER,
    Compute = GL_COMPUTE_SHADER,
    Unknown = GL_INVALID_ENUM,
};

struct ShaderInfo {
    FileInfo file;
};

class Shader {
  public:
    Shader(const ShaderInfo &info, const ShaderType &type);
    ~Shader();

    void build();
    void destroy();

    bool compile();

    void bind(GLuint program) const;
    void unbind(GLuint program) const;

    const GLuint &getShader() const;
    const ShaderType &getType() const;

  private:
    ShaderInfo m_Info;
    GLuint m_Shader;
    ShaderType m_Type;
};

struct ShaderPipelineInfo {
    ShaderInfo vertex;
    ShaderInfo tessControl;
    ShaderInfo tessEvaluation;
    ShaderInfo geometry;
    ShaderInfo fragment;
    ShaderInfo compute;
};

class ShaderPipelineResource {
  public:
    ShaderPipelineResource(const ShaderPipelineInfo &info);
    ~ShaderPipelineResource();

    void build();
    void destroy();

    void bind() const;
    void unbind() const;

    template <UniformType T>
    void setUniform(const std::string &name, const T &data) const {
        setUniformInternal(glGetUniformLocation(m_Program, name.c_str()), data);
    }

    const GLuint &getProgram() const;

  private:
    void setUniformInternal(const GLint location, const float &data) const;
    void setUniformInternal(const GLint location, const glm::vec2 &data) const;
    void setUniformInternal(const GLint location, const glm::vec3 &data) const;
    void setUniformInternal(const GLint location, const glm::vec4 &data) const;

    void setUniformInternal(const GLint location, const int &data) const;
    void setUniformInternal(const GLint location, const glm::ivec2 &data) const;
    void setUniformInternal(const GLint location, const glm::ivec3 &data) const;
    void setUniformInternal(const GLint location, const glm::ivec4 &data) const;

    void setUniformInternal(const GLint location, const unsigned int &data) const;
    void setUniformInternal(const GLint location, const glm::uvec2 &data) const;
    void setUniformInternal(const GLint location, const glm::uvec3 &data) const;
    void setUniformInternal(const GLint location, const glm::uvec4 &data) const;

    void setUniformInternal(const GLint location, const glm::mat2 &data) const;
    void setUniformInternal(const GLint location, const glm::mat3 &data) const;
    void setUniformInternal(const GLint location, const glm::mat4 &data) const;

    void setUniformInternal(const GLint location, const glm::mat2x3 &data) const;
    void setUniformInternal(const GLint location, const glm::mat3x2 &data) const;

    void setUniformInternal(const GLint location, const glm::mat2x4 &data) const;
    void setUniformInternal(const GLint location, const glm::mat4x2 &data) const;

    void setUniformInternal(const GLint location, const glm::mat3x4 &data) const;
    void setUniformInternal(const GLint location, const glm::mat4x3 &data) const;

    ShaderPipelineInfo m_Info;
    GLuint m_Program;
};

} // namespace Physbuzz
