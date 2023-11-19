#include "shaders.hpp"

ShaderContext::ShaderContext(std::string vertex, std::string fragment) : vertex(Shader(vertex, GL_VERTEX_SHADER)),
                                                                         fragment(Shader(fragment, GL_FRAGMENT_SHADER)) {
    program = glCreateProgram();
}

ShaderContext::~ShaderContext() {
    glDeleteProgram(program);
}

GLuint ShaderContext::load() {
    GLint result = GL_FALSE;
    int log_length;

    vertex.load();
    fragment.load();

    // linking
    glAttachShader(program, vertex.shader);
    glAttachShader(program, fragment.shader);
    glLinkProgram(program);

    // checks
    glGetProgramiv(program, GL_LINK_STATUS, &result);
    glGetProgramiv(program, GL_INFO_LOG_LENGTH, &log_length);
    if (log_length > 0) {
        std::vector<char> error_message(log_length + 1);
        glGetProgramInfoLog(program, log_length, NULL, error_message.data());
        throw std::runtime_error("Shader linking failed!\n\n" + std::string(error_message.data()));
    }

    // cleanup
    glDetachShader(program, vertex.shader);
    glDetachShader(program, fragment.shader);

    glDeleteShader(vertex.shader);
    glDeleteShader(fragment.shader);

    return program;
}

Shader::Shader(std::string source, uint32_t type) : source(source) {
    shader = glCreateShader(type);
}

Shader::~Shader() {
    glDeleteShader(shader);
}

GLuint Shader::load() {
    GLint result = GL_FALSE;
    int log_length;

    // compile (maybe use spir-v in the future?)
    const char *p_source = source.c_str();
    glShaderSource(shader, 1, &p_source, NULL);
    glCompileShader(shader);

    // checks
    glGetShaderiv(shader, GL_COMPILE_STATUS, &result);
    glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &log_length);
    if (log_length > 0) {
        std::vector<char> error_message(log_length + 1);
        glGetShaderInfoLog(shader, log_length, NULL, error_message.data());
        throw std::runtime_error("Shader compilation failed!\n\n" + std::string(error_message.data()));
    }

    return shader;
}
