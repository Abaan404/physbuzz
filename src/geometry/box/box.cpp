#include "box.hpp"

#include "../../renderer/renderer.hpp"
#include "../../renderer/shaders.hpp"
#include "shaders/box.frag"
#include "shaders/box.vert"
#include <glad/gl.h>

Box::Box(glm::vec2 position, float width, float height, float mass) : GameObject(Objects::Box, position), width(width), height(height) {
    min = glm::vec2(position[0] - (width / 2.0f), position[1] - (height / 2.0f));
    max = glm::vec2(position[0] + (width / 2.0f), position[1] + (height / 2.0f));

    static ShaderContext shader = ShaderContext(box_vertex, box_frag);

    GameObject::texture = std::make_unique<TextureBox>(*this, shader.program);
    GameObject::dynamics = std::make_unique<DynamicBox>(*this, mass);
};

TextureBox::TextureBox(Box &box, unsigned int &program) : TextureObject(program), box(box) {
    u_color = glGetUniformLocation(program, "u_color");
}

void TextureBox::draw(Renderer &renderer, unsigned int usage) {
    std::vector<glm::vec3> vertex_buffer = {
        glm::vec3(box.min.x, box.min.y, 0), // top-left
        glm::vec3(box.min.x, box.max.y, 0), // top-right
        glm::vec3(box.max.x, box.min.y, 0), // bottom-left
        glm::vec3(box.max.x, box.max.y, 0), // bottom-right
    };

    for (int i = 0; i < vertex_buffer.size(); i++) {
        vertex_buffer[i] = renderer.screen_to_world(vertex_buffer[i]);
    }

    std::vector<glm::uvec3> index_buffer = {
        glm::vec3(0, 1, 2),
        glm::vec3(1, 2, 3),
    };

    glUseProgram(program);
    glUniform4f(u_color, 0.5f, 0.0f, 0.0f, 0.0f);
    TextureObject::set_vertex(vertex_buffer);
    TextureObject::set_index(index_buffer);
    TextureObject::draw(renderer, usage);
}

DynamicBox::DynamicBox(Box &box, float mass) : DynamicObject(mass), box(box) {}

void DynamicBox::tick() {
    box.position += velocity + 0.5f * acceleration;

    box.min = glm::vec2(box.position[0] - (box.width / 2.0f), box.position[1] - (box.height / 2.0f));
    box.max = glm::vec2(box.position[0] + (box.width / 2.0f), box.position[1] + (box.height / 2.0f));
}
