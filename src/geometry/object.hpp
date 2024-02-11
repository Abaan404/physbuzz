#pragma once

#include <glm/glm.hpp>
#include <vector>

class Renderer;

enum class Objects {
    Unknown = -1,
    Box,
    Circle,
    Triangle,
    Bezier,
    Polygon,
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
    std::vector<float> vertices;
    std::vector<unsigned int> indices;

    glm::vec2 position;
    float rotation;
    AABB rect;

    DynamicProperties dynamics;
    Objects identifier = Objects::Unknown;

    virtual void draw(Renderer *renderer, unsigned int usage);

    void set_program(unsigned int program);

  protected:
    unsigned int program;

    // uniforms
    int glu_time;
    int glu_timedelta;
    int glu_resolution;

    void set_vertex(std::vector<glm::vec3> vertex_buffer);
    void set_index(std::vector<glm::uvec3> index_buffer);
};
