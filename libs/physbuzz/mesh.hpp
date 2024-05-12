#pragma once

#include <glad/gl.h>
#include <glm/glm.hpp>
#include <vector>

namespace Physbuzz {

struct MeshComponent {
    std::vector<glm::uvec3> indices;
    std::vector<glm::vec3> actualVertices;
    std::vector<glm::vec3> screenVertices;
    std::vector<glm::vec3> orignalVertices;

    bool scaled = false;
};

class RenderComponent {
  public:
    RenderComponent();
    RenderComponent(const RenderComponent &other);
    RenderComponent &operator=(const RenderComponent &other);
    ~RenderComponent();

    void build();
    void destroy();

    GLuint getProgram() const;
    void setProgram(GLuint program);

    // TODO uniforms class abstraction
    GLint gluTime;
    GLint gluTimedelta;
    GLint gluResolution;

  private:
    void copy(const RenderComponent &other);

    GLuint VBO, VAO, EBO;
    GLuint m_Program;

    friend class Renderer;
};

} // namespace Physbuzz
