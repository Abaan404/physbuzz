#pragma once

#include <glad/gl.h>
#include <glm/glm.hpp>
#include <vector>

namespace Physbuzz {

using Index = std::uint32_t;

struct Vertex {
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec2 texCoords;
};

struct MeshInfo {
    std::vector<Vertex> vertices;
    std::vector<Index> indices;
};

class Mesh {
  public:
    Mesh(const MeshInfo &info);

    bool build();
    bool destroy();

    void bind() const;
    void draw() const;
    void unbind() const;

    void update(const MeshInfo &info);
    const MeshInfo &getInfo() const;

  private:
    GLuint VBO, VAO, EBO;

    MeshInfo m_Info;
};

} // namespace Physbuzz
