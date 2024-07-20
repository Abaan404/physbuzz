#pragma once

#include <glad/gl.h>

namespace Physbuzz {

class Shader {
  public:
    Shader(const GLchar *source, GLuint type);
    ~Shader();

    void build();
    void destroy();

    void bind(GLuint program) const;
    void unbind(GLuint program) const;

    GLuint getShader() const;

  private:
    const GLchar *m_Source;
    GLuint m_Type;
    GLuint m_Shader;
};

// TODO support more graphics pipelines
class ShaderPipeline {
  public:
    ShaderPipeline(const GLchar *vertex, const GLchar *fragment);
    ~ShaderPipeline();

    void build();
    void destroy();

    void bind() const;
    void unbind() const;

    GLuint getProgram() const;

  private:
    Shader m_Vertex;
    Shader m_Fragment;
    GLuint m_Program;
};

} // namespace Physbuzz
