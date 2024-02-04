#pragma once

#include <glm/glm.hpp>
#include <vector>

enum class Objects {
    Unknown = -1,
    Box,
    PhysicsBox,
    Circle,
    PhysicsCircle
};

struct DynamicProperties {
    float mass = 0.0f;
    float intertia = 0.0f;
    glm::vec2 velocity = glm::vec2(0.0f, 0.0f);
    glm::vec2 acceleration = glm::vec2(0.0f, 0.0f);
};

struct AABB {
    float x;
    float y;
    float w;
    float h;
};

class GameObject {
  public:
    GameObject(Objects identifier, glm::vec2 position);

    ~GameObject();

    unsigned int VBO, VAO, EBO;
    unsigned int program;

    glm::vec2 position;
    float rotation;
    std::vector<float> vertices;
    std::vector<unsigned int> indices;

    AABB rect;

    DynamicProperties dynamics;
    Objects identifier = Objects::Unknown;

    void set_vertex(std::vector<glm::vec3> vertex_buffer);
    void set_index(std::vector<glm::uvec3> index_buffer);
};
