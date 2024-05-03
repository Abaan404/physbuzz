#include "renderer.hpp"

#include "debug.hpp"
#include "window.hpp"
#include <GLFW/glfw3.h>
#include <glad/gl.h>

namespace Physbuzz {

Renderer::Renderer(Window &window) : m_Window(window) {
    target(nullptr);
}

Renderer::Renderer(const Renderer &other) : m_Window(other.m_Window) {
    if (this != &other) {
        target(other.m_Framebuffer);
    }
}

Renderer Renderer::operator=(const Renderer &other) {
    if (this != &other) {
        target(other.m_Framebuffer);
    }

    return *this;
}

Renderer::~Renderer() {}

Window &Renderer::getWindow() const {
    return m_Window;
}

glm::vec2 Renderer::getResolution() const {
    return m_Resolution;
}

void Renderer::target(Framebuffer *framebuffer) {
    m_Framebuffer = framebuffer;

    glm::ivec2 resolution;
    if (framebuffer == nullptr) {
        framebuffer->unbind();
        resolution = m_Window.getResolution();
    } else {
        framebuffer->bind();
        resolution = framebuffer->getResolution();
    }

    resize(resolution);
}

void Renderer::clear(glm::vec4 &color) {
    m_Framebuffer->clear(color);
}

void Renderer::resize(glm::ivec2 &resolution) {
    if (m_Framebuffer == nullptr) {
        m_Window.setResolution(resolution);
    } else {
        m_Framebuffer->resize(resolution);
    }

    m_Resolution = resolution;
    glViewport(0, 0, resolution.x, resolution.y);
}

void Renderer::normalize(MeshComponent &mesh) {
    ASSERT(mesh.m_ScreenVertices.size() % 3 == 0, "Invalid Vertex Length");

    int i = 0;
    mesh.m_Vertices.resize(mesh.m_ScreenVertices.size());
    while (i != mesh.m_ScreenVertices.size()) {
        mesh.m_Vertices[i++] = (2.0f * mesh.m_ScreenVertices[i]) / m_Resolution.x - 1.0f; // x
        mesh.m_Vertices[i++] = 1.0f - (2.0f * mesh.m_ScreenVertices[i]) / m_Resolution.y; // y
        mesh.m_Vertices[i++] = mesh.m_ScreenVertices[i];                                  // z
    }
}

void Renderer::render(Scene &scene) {
    std::vector<MeshComponent> &meshes = scene.getComponents<MeshComponent>();
    for (auto &mesh : meshes) {
        render(mesh);
    }
}

// way too many draw calls i know
void Renderer::render(MeshComponent &mesh) {
    time = m_Window.getTime();

    if (mesh.m_ScreenVertices.size() != mesh.m_Vertices.size()) {
        normalize(mesh);
    }

    // send data to the gpu
    glBindBuffer(GL_ARRAY_BUFFER, mesh.VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * mesh.m_Vertices.size(), mesh.m_Vertices.data(), GL_STREAM_DRAW);

    glBindVertexArray(mesh.VAO);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * mesh.m_Indices.size(), mesh.m_Indices.data(), GL_STREAM_DRAW);

    glUseProgram(mesh.m_Program);
    glUniform1ui(mesh.gluTime, time);
    glUniform2f(mesh.gluResolution, m_Resolution.x, m_Resolution.y);

    // draw object on screen
    glDrawElements(GL_TRIANGLES, mesh.m_Indices.size(), GL_UNSIGNED_INT, 0);

    // cleanup
    glBindVertexArray(0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

} // namespace Physbuzz
