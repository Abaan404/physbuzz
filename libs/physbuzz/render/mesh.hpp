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

class Mesh {
  public:
    struct Info {
        std::vector<Vertex> vertices;
        std::vector<Index> indices;
    };

    Mesh(const Info &info);

    bool build();
    bool destroy();

    void bind() const;
    void draw() const;
    void unbind() const;

    void update(const Info &info);
    const Info &getInfo() const;

  private:
    GLuint VBO, VAO, EBO;

    Info m_Info;
};

} // namespace Physbuzz
