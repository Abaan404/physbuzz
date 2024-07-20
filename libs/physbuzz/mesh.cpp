#include "mesh.hpp"

#include "glad/gl.h"
#include "glm/fwd.hpp"
#include "logging.hpp"
#include <cstddef>
#include <format>
#include <glm/gtc/type_ptr.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

namespace Physbuzz {

Mesh::Mesh() {}

Mesh::~Mesh() {}

void Mesh::build() {
    Logger::ASSERT(vertices.size() == positions.size(), "Incorrect Mesh Position Vertices");

    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);
    glGenVertexArrays(1, &VAO);

    // for (auto &texture : textures) {
    //     texture.build();
    // }
}

void Mesh::destroy() {
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
    glDeleteVertexArrays(1, &VAO);

    // for (auto &texture : textures) {
    //     texture.destroy();
    // }
}

void Mesh::bind() const {
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * vertices.size(), vertices.data(), GL_STREAM_DRAW);

    glBindVertexArray(VAO);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)(offsetof(Vertex, position)));
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)(offsetof(Vertex, normal)));
    glEnableVertexAttribArray(1);

    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)(offsetof(Vertex, texCoords)));
    glEnableVertexAttribArray(2);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices) * indices.size(), indices.data(), GL_STREAM_DRAW);
}

void Mesh::draw() const {
    glDrawElements(GL_TRIANGLES, indices.size() * 3, GL_UNSIGNED_INT, 0);
}

void Mesh::unbind() const {
    glBindVertexArray(0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

Texture::Texture(const std::string &path)
    : m_Path(path) {}

Texture::~Texture() {}

void Texture::build() {
    unsigned char *data = stbi_load(m_Path.c_str(), &m_Resolution.x, &m_Resolution.y, &m_Channels, 0);

    if (!data) {
        Logger::WARNING(std::format("Could not generate Texture2D \"{}\"", m_Path));
        unsigned char *data = stbi_load("assets/missing.png", &m_Resolution.x, &m_Resolution.y, &m_Channels, 0);
    }

    glGenTextures(1, &m_Id);
    glBindTexture(GL_TEXTURE_2D, m_Id);

    // Texture Wrapping (Repeat)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);

    // Mipmaps
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, m_Resolution.x, m_Resolution.y, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);

    stbi_image_free(data);
}

void Texture::destroy() {
    glDeleteTextures(1, &m_Id);
}

const glm::ivec2 &Texture::getResolution() const {
    return m_Resolution;
}

const int &Texture::getChannels() const {
    return m_Channels;
}

const GLuint &Texture::getId() const {
    return m_Id;
}

} // namespace Physbuzz
