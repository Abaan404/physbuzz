#pragma once

#include <glad/gl.h>
#include <stdexcept>
#include <string>
#include <vector>

class Shader {
  public:
    Shader(std::string source, uint32_t type);
    ~Shader();

    GLuint shader;

    GLuint load();

  private:
    std::string source;
};

struct ShaderContext {
    ShaderContext(std::string vertex, std::string fragment);
    ~ShaderContext();

    GLuint program;
    Shader vertex;
    Shader fragment;

    GLuint load();
};

// TODO util class to create and manage VBO, VAO, etc
class ShaderHelper {

};
