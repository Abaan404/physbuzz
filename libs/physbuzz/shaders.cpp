#include "shaders.hpp"

#include <iostream>
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

        std::cerr
            << "Shader Linking failed!\n"
            << std::string(errorMessage.data()) << std::endl;
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
        std::cerr
            << "Shader compilation failed!\n"
            << std::string(errorMessage.data()) << std::endl;
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
