#pragma once

#include <glad/gl.h>
#include <string>

class Shader {
  public:
    Shader(std::string source, unsigned int type);
    ~Shader();

    unsigned int shader;

    unsigned int compile();

  private:
    std::string source;
};

class ShaderContext {
  public:
    ShaderContext(std::string vertex, std::string fragment);
    ~ShaderContext();

    unsigned int program;

    unsigned int load();

  private:
    Shader vertex;
    Shader fragment;
};

// TODO util class to create and manage VBO, VAO, etc
class ShaderHelper {

};
