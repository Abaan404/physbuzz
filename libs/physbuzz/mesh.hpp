#pragma once

#include <glad/gl.h>
#include <glm/glm.hpp>
#include <vector>

namespace Physbuzz {

class RenderComponent;

class MeshComponent {
  public:
    MeshComponent(std::vector<glm::uvec3> indices, std::vector<glm::vec3> vertices);
    ~MeshComponent();

    const std::vector<glm::vec3> &getOriginalVertices() const;
    void renormalize();

    std::vector<glm::uvec3> indices;
    std::vector<glm::vec3> vertices;
    std::vector<glm::vec3> normals;

  private:
    std::vector<glm::vec3> m_NormalizedVertices;
    std::vector<glm::vec3> m_OriginalVertices;

    bool m_Scaled = false;

    friend class Renderer;
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
