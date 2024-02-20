#pragma once

#include <glm/glm.hpp>
#include <vector>

class Renderer;

class TextureObject {
  public:
    TextureObject(unsigned int &program);
    ~TextureObject();

    unsigned int VBO, VAO, EBO;
    std::vector<float> vertices;
    std::vector<unsigned int> indices;

    float rotation;

    virtual void draw(Renderer &renderer);

  protected:
    unsigned int &program;

    // uniforms
    int glu_time;
    int glu_timedelta;
    int glu_resolution;

    glm::vec3 normalize_vertex(Renderer &renderer, glm::vec3 vertex);
    void set_program(unsigned int program);
    void set_vertex(std::vector<glm::vec3> vertex_buffer);
    void set_index(std::vector<glm::uvec3> index_buffer);
};
