#include "box.hpp"

#include "../../renderer/renderer.hpp"
#include "../../renderer/shaders.hpp"
#include "shaders/box.frag"
#include "shaders/box.vert"

#include <glad/gl.h>

Box::Box(glm::vec2 position, float width, float height, float mass) : GameObject(Objects::Box, position) {
    min = glm::vec2(position[0] - (width / 2.0f), position[1] - (height / 2.0f));
    max = glm::vec2(position[0] + (width / 2.0f), position[1] + (height / 2.0f));

    // reuse the same shader
    static ShaderContext shader = ShaderContext(box_vertex, box_frag);
    u_color = glGetUniformLocation(shader.program, "u_color");

    GameObject::set_program(shader.program);
    GameObject::dynamics.mass = mass;
    GameObject::rect = {min.x, min.y, width, height};
};

void Box::draw(Renderer *renderer, unsigned int usage) {
    std::vector<glm::vec3> vertex_buffer = {
        glm::vec3(rect.x, rect.y, 0),                   // bottom-left
        glm::vec3(rect.x + rect.w, rect.y, 0),          // bottom-right
        glm::vec3(rect.x, rect.y + rect.h, 0),          // top-left
        glm::vec3(rect.x + rect.w, rect.y + rect.h, 0), // top-right
    };

    for (int i = 0; i < vertex_buffer.size(); i++) {
        vertex_buffer[i] = renderer->screen_to_world(vertex_buffer[i]);
    }

    std::vector<glm::uvec3> index_buffer = {
        glm::vec3(0, 1, 2),
        glm::vec3(1, 2, 3),
    };

    glUseProgram(program);
    glUniform4f(u_color, 0.5, 0.0, 0.0, 0.0);
    GameObject::set_vertex(vertex_buffer);
    GameObject::set_index(index_buffer);
    GameObject::draw(renderer, usage);
}
