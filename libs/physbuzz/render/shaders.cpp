#include "shaders.hpp"

#include "../debug/logging.hpp"
#include <bitset>
#include <format>
#include <vector>

namespace Physbuzz {

Shader::Shader(const ShaderInfo &info, const ShaderType &type)
    : m_Info(info), m_Type(type) {}

Shader::~Shader() {}

void Shader::build() {
    m_Shader = glCreateShader(static_cast<GLuint>(m_Type));
}

void Shader::destroy() {
    glDeleteShader(m_Shader);
}

bool Shader::compile() {
    if (m_Info.file.path.empty()) {
        return false;
    }

    FileResource file = FileResource(m_Info.file);
    file.build();
    file.read();

    if (file.buffer.empty()) {
        file.destroy();
        return false;
    }

    const char *source = file.buffer.data();
    glShaderSource(m_Shader, 1, &source, NULL);
    glCompileShader(m_Shader);
    file.destroy();

    GLint result;
    glGetShaderiv(m_Shader, GL_COMPILE_STATUS, &result);
    if (result == GL_FALSE) {

        GLint logLength;
        glGetShaderiv(m_Shader, GL_INFO_LOG_LENGTH, &logLength);

        std::vector<char> errorMessage(logLength + 1);
        glGetShaderInfoLog(m_Shader, logLength, NULL, errorMessage.data());

        Logger::ERROR(std::format("[Shader] Shader compilation failed!\n{}", errorMessage.data()));
        return false;
    }

    return true;
}

void Shader::bind(GLuint program) const {
    glAttachShader(program, m_Shader);
}

void Shader::unbind(GLuint program) const {
    glDetachShader(program, m_Shader);
}

const GLuint &Shader::getShader() const {
    return m_Shader;
}

const ShaderType &Shader::getType() const {
    return m_Type;
}

ShaderPipelineResource::ShaderPipelineResource(const ShaderPipelineInfo &info)
    : m_Info(info) {}

ShaderPipelineResource::~ShaderPipelineResource() {}

void ShaderPipelineResource::build() {
    std::array shaders = {
        Shader(m_Info.vertex, ShaderType::Vertex),
        Shader(m_Info.tessControl, ShaderType::TessControl),
        Shader(m_Info.tessEvaluation, ShaderType::TessEvaluation),
        Shader(m_Info.geometry, ShaderType::Geometry),
        Shader(m_Info.fragment, ShaderType::Fragment),
        Shader(m_Info.compute, ShaderType::Compute),
    };

    std::bitset<sizeof(ShaderPipelineInfo) / sizeof(ShaderInfo)> compiled = 0;

    m_Program = glCreateProgram();
    for (int i = 0; i < shaders.size(); i++) {
        shaders[i].build();

        if (shaders[i].compile()) {
            compiled[i] = true;
            shaders[i].bind(m_Program);
        }
    }

    if (compiled[offsetof(ShaderPipelineInfo, compute) / sizeof(ShaderInfo)]) {
        Logger::ERROR("[ShaderPipelineResource] Compute shaders are not supported by the engine.");
        return;
    }

    if (!compiled[offsetof(ShaderPipelineInfo, vertex) / sizeof(ShaderInfo)]) {
        Logger::WARNING("[ShaderPipelineResource] Could not compile vertex shader");
        return;
    }

    if (!compiled[offsetof(ShaderPipelineInfo, fragment) / sizeof(ShaderInfo)]) {
        Logger::WARNING("[ShaderPipelineResource] Could not compile fragment shader.");
        return;
    }

    glLinkProgram(m_Program);
    glValidateProgram(m_Program);

    GLint result;
    glGetProgramiv(m_Program, GL_LINK_STATUS, &result);
    if (result == GL_FALSE) {
        GLint logLength;
        glGetProgramiv(m_Program, GL_INFO_LOG_LENGTH, &logLength);

        std::vector<char> errorMessage(logLength + 1);
        glGetProgramInfoLog(m_Program, logLength, NULL, errorMessage.data());

        Logger::ERROR(std::format("[ShaderPipelineResource] Shader Linking failed!\n{}", errorMessage.data()));
    }

    for (int i = 0; i < shaders.size(); i++) {
        if (compiled[i]) {
            shaders[i].unbind(m_Program);
        }

        shaders[i].destroy();
    }
}

void ShaderPipelineResource::destroy() {
    glDeleteProgram(m_Program);
}

void ShaderPipelineResource::bind() const {
    glUseProgram(m_Program);
}

void ShaderPipelineResource::unbind() const {
    glUseProgram(0); // Undefined Behaviour, upto the user to use the right shader
}

const GLuint &ShaderPipelineResource::getProgram() const {
    return m_Program;
}

void ShaderPipelineResource::setUniformInternal(const GLint location, const float &data) const { glUniform1fv(location, 1, &data); }
void ShaderPipelineResource::setUniformInternal(const GLint location, const glm::vec2 &data) const { glUniform2fv(location, 1, &data[0]); }
void ShaderPipelineResource::setUniformInternal(const GLint location, const glm::vec3 &data) const { glUniform3fv(location, 1, &data[0]); }
void ShaderPipelineResource::setUniformInternal(const GLint location, const glm::vec4 &data) const { glUniform4fv(location, 1, &data[0]); }

void ShaderPipelineResource::setUniformInternal(const GLint location, const int &data) const { glUniform1iv(location, 1, &data); }
void ShaderPipelineResource::setUniformInternal(const GLint location, const glm::ivec2 &data) const { glUniform2iv(location, 1, &data[0]); }
void ShaderPipelineResource::setUniformInternal(const GLint location, const glm::ivec3 &data) const { glUniform3iv(location, 1, &data[0]); }
void ShaderPipelineResource::setUniformInternal(const GLint location, const glm::ivec4 &data) const { glUniform4iv(location, 1, &data[0]); }

void ShaderPipelineResource::setUniformInternal(const GLint location, const unsigned int &data) const { glUniform1uiv(location, 1, &data); }
void ShaderPipelineResource::setUniformInternal(const GLint location, const glm::uvec2 &data) const { glUniform2uiv(location, 1, &data[0]); }
void ShaderPipelineResource::setUniformInternal(const GLint location, const glm::uvec3 &data) const { glUniform3uiv(location, 1, &data[0]); }
void ShaderPipelineResource::setUniformInternal(const GLint location, const glm::uvec4 &data) const { glUniform4uiv(location, 1, &data[0]); }

void ShaderPipelineResource::setUniformInternal(const GLint location, const glm::mat2 &data) const { glUniformMatrix2fv(location, 1, GL_FALSE, &data[0][0]); }
void ShaderPipelineResource::setUniformInternal(const GLint location, const glm::mat3 &data) const { glUniformMatrix3fv(location, 1, GL_FALSE, &data[0][0]); }
void ShaderPipelineResource::setUniformInternal(const GLint location, const glm::mat4 &data) const { glUniformMatrix4fv(location, 1, GL_FALSE, &data[0][0]); }

void ShaderPipelineResource::setUniformInternal(const GLint location, const glm::mat2x3 &data) const { glUniformMatrix2x3fv(location, 1, GL_FALSE, &data[0][0]); }
void ShaderPipelineResource::setUniformInternal(const GLint location, const glm::mat3x2 &data) const { glUniformMatrix3x2fv(location, 1, GL_FALSE, &data[0][0]); }

void ShaderPipelineResource::setUniformInternal(const GLint location, const glm::mat2x4 &data) const { glUniformMatrix2x4fv(location, 1, GL_FALSE, &data[0][0]); }
void ShaderPipelineResource::setUniformInternal(const GLint location, const glm::mat4x2 &data) const { glUniformMatrix4x2fv(location, 1, GL_FALSE, &data[0][0]); }

void ShaderPipelineResource::setUniformInternal(const GLint location, const glm::mat3x4 &data) const { glUniformMatrix3x4fv(location, 1, GL_FALSE, &data[0][0]); }
void ShaderPipelineResource::setUniformInternal(const GLint location, const glm::mat4x3 &data) const { glUniformMatrix4x3fv(location, 1, GL_FALSE, &data[0][0]); }

} // namespace Physbuzz
