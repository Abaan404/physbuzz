#include "renderer.hpp"

#include "logging.hpp"
#include "window.hpp"
#include <GLFW/glfw3.h>
#include <glad/gl.h>
#include <glm/gtc/type_ptr.hpp>

// pointer magic
template <typename T, glm::length_t N>
std::pair<T *, std::size_t> getArray(std::vector<glm::vec<N, T>> &vec) {
    return {static_cast<T *>(glm::value_ptr(vec.front())), vec.size() * (vec.front().length())};
}

namespace Physbuzz {

Renderer::Renderer(Window &window) : m_Window(window) {}

Renderer::Renderer(const Renderer &other) : m_Window(other.m_Window) {
    if (this != &other) {
        target(other.m_Framebuffer);
    }
}

Renderer &Renderer::operator=(const Renderer &other) {
    if (this != &other) {
        target(other.m_Framebuffer);
    }

    return *this;
}

Renderer::~Renderer() {}

void Renderer::build() {
    target(nullptr);
}

void Renderer::destroy() {}

Window &Renderer::getWindow() const {
    return m_Window;
}

glm::ivec2 Renderer::getResolution() const {
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
    mesh.m_NormalizedVertices.clear();
    mesh.m_Scaled = true;
    for (auto &vertex : mesh.vertices) {
        mesh.m_NormalizedVertices.emplace_back((2.0f * vertex.x) / m_Resolution.x - 1.0f, 1.0f - (2.0f * vertex.y) / m_Resolution.y, vertex.z);
    }
}

void Renderer::render(Scene &scene) {
    for (auto &object : scene.getObjects()) {
        if (!(object.hasComponent<RenderComponent>())) {
            continue;
        }

        Logger::ASSERT(object.hasComponent<MeshComponent>(), "RenderComponent object does not have a mesh!");
        render(object);
    }
}

// way too many draw calls i know
void Renderer::render(Object &object) {
    time = m_Window.getTime();

    RenderComponent &render = object.getComponent<RenderComponent>();
    MeshComponent &mesh = object.getComponent<MeshComponent>();

    if (!mesh.m_Scaled) {
        normalize(mesh);
    }

    // auto vertex = getArray<float, 3>(component.m_Mesh.vertices);
    auto [vertices, verticesSize] = getArray<float, 3>(mesh.m_NormalizedVertices);
    auto [indices, indicesSize] = getArray<std::uint32_t, 3>(mesh.indices);

    // send data to the gpu
    glBindBuffer(GL_ARRAY_BUFFER, render.VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * verticesSize, vertices, GL_STREAM_DRAW);

    glBindVertexArray(render.VAO);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, render.EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * indicesSize, indices, GL_STREAM_DRAW);

    glUseProgram(render.m_Program);
    glUniform1ui(render.gluTime, time);
    glUniform2f(render.gluResolution, m_Resolution.x, m_Resolution.y);

    // draw object on screen
    glDrawElements(GL_TRIANGLES, indicesSize, GL_UNSIGNED_INT, 0);

    // cleanup
    glBindVertexArray(0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

} // namespace Physbuzz
