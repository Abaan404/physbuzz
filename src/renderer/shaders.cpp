#include "shaders.hpp"

#include <glad/gl.h>
#include <iostream>
#include <vector>

ShaderContext::ShaderContext(std::string vertex, std::string fragment) : vertex(Shader(vertex, GL_VERTEX_SHADER)),
                                                                         fragment(Shader(fragment, GL_FRAGMENT_SHADER)) {
    program = glCreateProgram();
    load();
}

ShaderContext::~ShaderContext() {
    glDeleteProgram(program);
}

unsigned int ShaderContext::load() {
    int result;

    vertex.compile();
    fragment.compile();

    // linking
    glAttachShader(program, vertex.shader);
    glAttachShader(program, fragment.shader);
    glLinkProgram(program);
    glValidateProgram(program);

    // checks
    glGetProgramiv(program, GL_LINK_STATUS, &result);
    if (result == GL_FALSE) {
        int log_length;
        glGetProgramiv(program, GL_INFO_LOG_LENGTH, &log_length);

        std::vector<char> error_message(log_length + 1);
        glGetProgramInfoLog(program, log_length, NULL, error_message.data());

        std::cerr << "Shader Linking failed!\n"
                  << std::string(error_message.data())
                  << std::endl;

        return 0;
    }

    // cleanup
    glDetachShader(program, vertex.shader);
    glDetachShader(program, fragment.shader);

    glDeleteShader(vertex.shader);
    glDeleteShader(fragment.shader);

    return program;
}

Shader::Shader(std::string source, unsigned int type) : source(source) {
    shader = glCreateShader(type);
}

Shader::~Shader() {
    glDeleteShader(shader);
}

unsigned int Shader::compile() {
    int result;

    // compile (maybe use spir-v in the future?)
    const char *p_source = source.c_str();
    glShaderSource(shader, 1, &p_source, NULL);
    glCompileShader(shader);

    // checks
    glGetShaderiv(shader, GL_COMPILE_STATUS, &result);
    if (result == GL_FALSE) {
        int log_length;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &log_length);

        std::vector<char> error_message(log_length + 1);
        glGetShaderInfoLog(shader, log_length, NULL, error_message.data());
        std::cerr << "Shader compilation failed!\n"
                  << std::string(error_message.data())
                  << std::endl;

        return 0;
    }

    return shader;
}
