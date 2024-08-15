#pragma once

#include <glad/gl.h>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <vector>

namespace Physbuzz {

using Index = glm::uvec3;

struct Vertex {
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec2 texCoords;
};

struct Model {
    void update();
    void reset();

    const glm::vec3 toWorld(const glm::vec3 &local) const;
    const glm::vec3 toLocal(const glm::vec3 &world) const;

    glm::vec3 position = {0.0f, 0.0f, 0.0f};
    glm::vec3 scale = {1.0f, 1.0f, 1.0f};
    glm::quat orientation = glm::angleAxis(0.0f, glm::vec3(0.0f, 0.0f, 1.0f));

    glm::mat4 matrix = glm::mat4(1.0f);
};

class MeshComponent {
  public:
    MeshComponent(const Model &model);
    ~MeshComponent();

    void build();
    void destroy();

    void bind() const;
    void draw() const;
    void unbind() const;

    std::vector<Vertex> vertices;
    std::vector<Index> indices;

    Model model;

  private:
    GLuint VBO, VAO, EBO;
};

} // namespace Physbuzz
