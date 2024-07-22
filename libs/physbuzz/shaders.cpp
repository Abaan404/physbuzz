#include "shaders.hpp"

#include "logging.hpp"
#include <format>
#include <vector>

namespace Physbuzz {

ShaderPipeline::ShaderPipeline(const GLchar *vertex, const GLchar *fragment)
    : m_Vertex(Shader(vertex, GL_VERTEX_SHADER)),
      m_Fragment(Shader(fragment, GL_FRAGMENT_SHADER)) {}

ShaderPipeline::~ShaderPipeline() {}

void ShaderPipeline::build() {
    m_Program = glCreateProgram();

    m_Vertex.build();
    m_Fragment.build();

    m_Vertex.bind(m_Program);
    m_Fragment.bind(m_Program);

    // linking
    glLinkProgram(m_Program);
    glValidateProgram(m_Program);

    // checks
    GLint result;
    glGetProgramiv(m_Program, GL_LINK_STATUS, &result);
    if (result == GL_FALSE) {
        GLint logLength;
        glGetProgramiv(m_Program, GL_INFO_LOG_LENGTH, &logLength);

        std::vector<char> errorMessage(logLength + 1);
        glGetProgramInfoLog(m_Program, logLength, NULL, errorMessage.data());

        Logger::ERROR(std::format("Shader Linking failed!\n{}", errorMessage.data()));
    }

    // cleanup
    m_Vertex.unbind(m_Program);
    m_Fragment.unbind(m_Program);

    m_Vertex.destroy();
    m_Fragment.destroy();
}

void ShaderPipeline::destroy() {
    glDeleteProgram(m_Program);
}

void ShaderPipeline::bind() const {
    glUseProgram(m_Program);
}

void ShaderPipeline::unbind() const {
    // glUseProgram(0); // Undefined Behaviour, upto the user to use the right shader
}

GLuint ShaderPipeline::getProgram() const {
    return m_Program;
}

void ShaderPipeline::setUniformInternal(const GLint location, const float &data) const { glUniform1fv(location, 1, &data); }
void ShaderPipeline::setUniformInternal(const GLint location, const glm::vec2 &data) const { glUniform2fv(location, 1, &data[0]); }
void ShaderPipeline::setUniformInternal(const GLint location, const glm::vec3 &data) const { glUniform3fv(location, 1, &data[0]); }
void ShaderPipeline::setUniformInternal(const GLint location, const glm::vec4 &data) const { glUniform4fv(location, 1, &data[0]); }

void ShaderPipeline::setUniformInternal(const GLint location, const int &data) const { glUniform1iv(location, 1, &data); }
void ShaderPipeline::setUniformInternal(const GLint location, const glm::ivec2 &data) const { glUniform2iv(location, 1, &data[0]); }
void ShaderPipeline::setUniformInternal(const GLint location, const glm::ivec3 &data) const { glUniform3iv(location, 1, &data[0]); }
void ShaderPipeline::setUniformInternal(const GLint location, const glm::ivec4 &data) const { glUniform4iv(location, 1, &data[0]); }

void ShaderPipeline::setUniformInternal(const GLint location, const unsigned int &data) const { glUniform1uiv(location, 1, &data); }
void ShaderPipeline::setUniformInternal(const GLint location, const glm::uvec2 &data) const { glUniform2uiv(location, 1, &data[0]); }
void ShaderPipeline::setUniformInternal(const GLint location, const glm::uvec3 &data) const { glUniform3uiv(location, 1, &data[0]); }
void ShaderPipeline::setUniformInternal(const GLint location, const glm::uvec4 &data) const { glUniform4uiv(location, 1, &data[0]); }

void ShaderPipeline::setUniformInternal(const GLint location, const glm::mat2 &data) const { glUniformMatrix2fv(location, 1, GL_FALSE, &data[0][0]); }
void ShaderPipeline::setUniformInternal(const GLint location, const glm::mat3 &data) const { glUniformMatrix3fv(location, 1, GL_FALSE, &data[0][0]); }
void ShaderPipeline::setUniformInternal(const GLint location, const glm::mat4 &data) const { glUniformMatrix4fv(location, 1, GL_FALSE, &data[0][0]); }

void ShaderPipeline::setUniformInternal(const GLint location, const glm::mat2x3 &data) const { glUniformMatrix2x3fv(location, 1, GL_FALSE, &data[0][0]); }
void ShaderPipeline::setUniformInternal(const GLint location, const glm::mat3x2 &data) const { glUniformMatrix3x2fv(location, 1, GL_FALSE, &data[0][0]); }

void ShaderPipeline::setUniformInternal(const GLint location, const glm::mat2x4 &data) const { glUniformMatrix2x4fv(location, 1, GL_FALSE, &data[0][0]); }
void ShaderPipeline::setUniformInternal(const GLint location, const glm::mat4x2 &data) const { glUniformMatrix4x2fv(location, 1, GL_FALSE, &data[0][0]); }

void ShaderPipeline::setUniformInternal(const GLint location, const glm::mat3x4 &data) const { glUniformMatrix3x4fv(location, 1, GL_FALSE, &data[0][0]); }
void ShaderPipeline::setUniformInternal(const GLint location, const glm::mat4x3 &data) const { glUniformMatrix4x3fv(location, 1, GL_FALSE, &data[0][0]); }

Shader::Shader(const GLchar *source, GLuint type)
    : m_Source(source), m_Type(type) {}

Shader::~Shader() {}

void Shader::build() {
    m_Shader = glCreateShader(m_Type);
    GLint result;

    // compile (maybe use spir-v in the future?)
    const GLchar *p_Source = m_Source;
    glShaderSource(m_Shader, 1, &p_Source, NULL);
    glCompileShader(m_Shader);

    // checks
    glGetShaderiv(m_Shader, GL_COMPILE_STATUS, &result);
    if (result == GL_FALSE) {
        GLint logLength;
        glGetShaderiv(m_Shader, GL_INFO_LOG_LENGTH, &logLength);

        std::vector<char> errorMessage(logLength + 1);
        glGetShaderInfoLog(m_Shader, logLength, NULL, errorMessage.data());

        Logger::ERROR(std::format("Shader compilation failed!\n{}", errorMessage.data()));
    }
}

void Shader::destroy() {
    glDeleteShader(m_Shader);
}

void Shader::bind(GLuint program) const {
    glAttachShader(program, m_Shader);
}

void Shader::unbind(GLuint program) const {
    glDetachShader(program, m_Shader);
}

GLuint Shader::getShader() const {
    return m_Shader;
}

} // namespace Physbuzz
