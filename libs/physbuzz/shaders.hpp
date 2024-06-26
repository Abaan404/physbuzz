#pragma once

#include <glad/gl.h>

namespace Physbuzz {

class Shader {
  public:
    Shader(const GLchar *source, GLuint type);
    ~Shader();

    void build();
    void destroy();

    GLuint shader;

    GLuint compile();

  private:
    const GLchar *m_Source;
    GLuint m_Type;
};

class ShaderContext {
  public:
    ShaderContext(const GLchar *vertex, const GLchar *fragment);
    ~ShaderContext();

    void build();
    void destroy();

    GLuint program;

    GLuint load();

  private:
    Shader m_Vertex;
    Shader m_Fragment;
};

} // namespace Physbuzz
