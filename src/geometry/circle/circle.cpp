#include "circle.hpp"

#include "../../renderer/renderer.hpp"
#include "../../renderer/shaders.hpp"
#include "shaders/circle.frag"
#include "shaders/circle.vert"

#include <glad/gl.h>

#define MAX_VERTICES 50

Circle::Circle(glm::vec2 position, float radius, float mass) : GameObject(Objects::Circle, position), radius(radius) {
    static ShaderContext shader = ShaderContext(circle_vertex, circle_frag);
    gl_color = glGetUniformLocation(shader.program, "u_color");

    GameObject::set_program(shader.program);
    GameObject::dynamics.mass = mass;
    GameObject::rect = {position.x - radius, position.y - radius, 2 * radius, 2 * radius};
}

void Circle::draw(Renderer *renderer, unsigned int usage) {
    std::vector<glm::vec3> vertex_buffer;
    std::vector<glm::uvec3> index_buffer;

    float angle_increment = (2.0f * M_PI) / MAX_VERTICES;

    // good ol trig
    for (int i = 0; i < MAX_VERTICES; i++) {
        float angle = i * angle_increment;
        float x = position.x + radius * glm::cos(angle);
        float y = position.y + radius * glm::sin(angle);
        vertex_buffer.push_back(glm::vec3(x, y, 0));
    }

    for (int i = 1; i < MAX_VERTICES - 1; i++) {
        index_buffer.push_back(glm::uvec3(0, i, i + 1));
    }
    index_buffer.push_back(glm::uvec3(0, MAX_VERTICES - 1, 1));

    for (int i = 0; i < vertex_buffer.size(); i++) {
        vertex_buffer[i] = renderer->screen_to_world(vertex_buffer[i]);
    }

    glUseProgram(program);
    glUniform4f(gl_color, 0.5, 0.0, 0.0, 0.0);
    GameObject::set_vertex(vertex_buffer);
    GameObject::set_index(index_buffer);
    GameObject::draw(renderer, usage);
}