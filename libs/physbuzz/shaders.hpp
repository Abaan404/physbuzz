#pragma once

namespace Physbuzz {

class Shader {
  public:
    Shader(const char* source, unsigned int type);
    ~Shader();

    unsigned int shader;

    unsigned int compile();

  private:
    const char* m_Source;
};

class ShaderContext {
  public:
    ShaderContext(const char* vertex, const char* fragment);
    ~ShaderContext();

    unsigned int program;

    unsigned int load();

  private:
    Shader m_Vertex;
    Shader m_Fragment;
};

} // namespace Physbuzz
