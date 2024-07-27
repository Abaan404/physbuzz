#pragma once

#include <glad/gl.h>
#include <glm/glm.hpp>
#include <vector>

namespace Physbuzz {

using Index = glm::uvec3;

struct Vertex {
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec2 texCoords;
};

class Mesh {
  public:
    Mesh();
    ~Mesh();

    void build();
    void destroy();

    void bind() const;
    void draw() const;
    void unbind() const;

    std::vector<Vertex> vertices;
    std::vector<Index> indices;

    std::vector<glm::vec3> positions;
    bool isScaled = false;

  private:
    GLuint VBO, VAO, EBO;
};

} // namespace Physbuzz
