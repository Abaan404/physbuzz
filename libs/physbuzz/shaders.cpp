#include "shaders.hpp"

#include <glad/gl.h>
#include <iostream>
#include <vector>

namespace Physbuzz {

ShaderContext::ShaderContext(const char* vertex, const char* fragment) : m_Vertex(Shader(vertex, GL_VERTEX_SHADER)),
                                                                         m_Fragment(Shader(fragment, GL_FRAGMENT_SHADER)) {
    program = glCreateProgram();
}

ShaderContext::~ShaderContext() {
    glDeleteProgram(program);
}

unsigned int ShaderContext::load() {
    int result;

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
        int logLength;
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

Shader::Shader(const char* source, unsigned int type) : m_Source(source) {
    shader = glCreateShader(type);
}

Shader::~Shader() {
    glDeleteShader(shader);
}

unsigned int Shader::compile() {
    int result;

    // compile (maybe use spir-v in the future?)
    const char *p_Source = m_Source;
    glShaderSource(shader, 1, &p_Source, NULL);
    glCompileShader(shader);

    // checks
    glGetShaderiv(shader, GL_COMPILE_STATUS, &result);
    if (result == GL_FALSE) {
        int logLength;
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
