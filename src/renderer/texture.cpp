#include "texture.hpp"

#include "renderer.hpp"
#include <glad/gl.h>

TextureObject::TextureObject(unsigned int &program) : program(program) {
    set_program(program);

    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);
    glGenVertexArrays(1, &VAO);
};

TextureObject::~TextureObject() {
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
    glDeleteVertexArrays(1, &VAO);
}

void TextureObject::set_vertex(std::vector<glm::vec3> vertex_buffer) {
    int idx = 0;
    vertices.resize(vertex_buffer.size() * 3);
    for (auto vertex = vertex_buffer.begin(); vertex != vertex_buffer.end(); vertex++) {
        vertices[idx++] = vertex->x;
        vertices[idx++] = vertex->y;
        vertices[idx++] = vertex->z;
    }
}

void TextureObject::set_index(std::vector<glm::uvec3> index_buffer) {
    int idx = 0;
    indices.resize(index_buffer.size() * 3);
    for (auto index = index_buffer.begin(); index != index_buffer.end(); index++) {
        indices[idx++] = index->x;
        indices[idx++] = index->y;
        indices[idx++] = index->z;
    }
}

void TextureObject::set_program(unsigned int program) {
    TextureObject::program = program;
    glUseProgram(program);

    // common uniforms for every shader
    glu_time = glGetUniformLocation(program, "u_time");
    glu_resolution = glGetUniformLocation(program, "u_resolution");
}

void TextureObject::draw(Renderer &renderer, GLenum usage) {
    // send data to the gpu
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * vertices.size(), vertices.data(), usage);

    glBindVertexArray(VAO);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * indices.size(), indices.data(), usage);

    glUniform1ui(glu_time, renderer.time);
    glUniform2f(glu_resolution, renderer.width, renderer.height);

    // draw object on screen
    glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);

    // cleanup
    glBindVertexArray(0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}
