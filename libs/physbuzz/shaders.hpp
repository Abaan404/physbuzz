#pragma once

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

class Shader {
  public:
    Shader(const GLchar *source, GLuint type);
    ~Shader();

    void build();
    void destroy();

    void bind(GLuint program) const;
    void unbind(GLuint program) const;

    GLuint getShader() const;

  private:
    const GLchar *m_Source;
    GLuint m_Type;
    GLuint m_Shader;
};

// TODO support more graphics pipelines
class ShaderPipeline {
  public:
    ShaderPipeline(const GLchar *vertex, const GLchar *fragment);
    ~ShaderPipeline();

    void build();
    void destroy();

    void bind() const;
    void unbind() const;

    template <typename T>
        requires UniformType<T>
    void setUniform(const std::string &name, const T &data) const {
        setUniformInternal(glGetUniformLocation(m_Program, name.c_str()), data);
    }

    GLuint getProgram() const;

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

    Shader m_Vertex;
    Shader m_Fragment;
    GLuint m_Program;
};

} // namespace Physbuzz
