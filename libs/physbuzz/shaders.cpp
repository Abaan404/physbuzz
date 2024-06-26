#include "shaders.hpp"

#include <iostream>
#include <vector>

namespace Physbuzz {

ShaderContext::ShaderContext(const GLchar *vertex, const GLchar *fragment)
    : m_Vertex(Shader(vertex, GL_VERTEX_SHADER)),
      m_Fragment(Shader(fragment, GL_FRAGMENT_SHADER)) {}

ShaderContext::~ShaderContext() {}

void ShaderContext::build() {
    m_Vertex.build();
    m_Fragment.build();

    program = glCreateProgram();
}

void ShaderContext::destroy() {
    m_Vertex.destroy();
    m_Fragment.destroy();

    glDeleteProgram(program);
}

GLuint ShaderContext::load() {
    GLint result;

    m_Vertex.compile();
    m_Fragment.compile();

    // linking
    glAttachShader(program, m_Vertex.shader);
    glAttachShader(program, m_Fragment.shader);
    glLinkProgram(program);
    glValidateProgram(program);

    // checks
    glGetProgramiv(program, GL_LINK_STATUS, &result);
    if (result == GL_FALSE) {
        GLint logLength;
        glGetProgramiv(program, GL_INFO_LOG_LENGTH, &logLength);

        std::vector<char> errorMessage(logLength + 1);
        glGetProgramInfoLog(program, logLength, NULL, errorMessage.data());

        std::cerr << "Shader Linking failed!\n"
                  << std::string(errorMessage.data())
                  << std::endl;

        return 0;
    }

    // cleanup
    glDetachShader(program, m_Vertex.shader);
    glDetachShader(program, m_Fragment.shader);

    glDeleteShader(m_Vertex.shader);
    glDeleteShader(m_Fragment.shader);

    return program;
}

Shader::Shader(const GLchar *source, GLuint type) : m_Source(source), m_Type(type) {}

Shader::~Shader() {}

void Shader::build() {
    shader = glCreateShader(m_Type);
}

void Shader::destroy() {
    glDeleteShader(shader);
}

GLuint Shader::compile() {
    GLint result;

    // compile (maybe use spir-v in the future?)
    const GLchar *p_Source = m_Source;
    glShaderSource(shader, 1, &p_Source, NULL);
    glCompileShader(shader);

    // checks
    glGetShaderiv(shader, GL_COMPILE_STATUS, &result);
    if (result == GL_FALSE) {
        GLint logLength;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logLength);

        std::vector<char> errorMessage(logLength + 1);
        glGetShaderInfoLog(shader, logLength, NULL, errorMessage.data());
        std::cerr << "Shader compilation failed!\n"
                  << std::string(errorMessage.data())
                  << std::endl;

        return 0;
    }

    return shader;
}

} // namespace Physbuzz
