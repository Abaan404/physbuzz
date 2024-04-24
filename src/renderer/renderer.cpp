#include "renderer.hpp"

#include <glad/gl.h>
#include <vector>

Renderer::Renderer(SDL_GLContext *context, SDL_Window *window, Scene &scene) : context(context),
                                                                               scene(scene),
                                                                               window(window) {
    target(nullptr);
}

void Renderer::render() {
    time = SDL_GetTicks();

    std::vector<MeshComponent> &meshes = scene.get_components<MeshComponent>();
    for (auto &mesh : meshes) {
        if (!mesh.normalized) {
            mesh.vertices = normalize_vertex(mesh);
        }

        render_mesh(mesh);
    }
}

void Renderer::target(Framebuffer *framebuffer) {
    this->framebuffer = framebuffer;

    if (framebuffer == nullptr) {
        framebuffer->unbind();
        SDL_GetWindowSize(window, &resolution.x, &resolution.y);
        return;
    }

    framebuffer->bind();
    resolution = framebuffer->resolution;
}

void Renderer::clear(glm::vec4 color) {
    framebuffer->clear(color);
}

void Renderer::resize(glm::ivec2 resolution) {
    if (framebuffer != nullptr) {
        framebuffer->resize(resolution);
    } else {
        SDL_GetWindowSize(window, &resolution.x, &resolution.y);
    }

    glViewport(0, 0, resolution.x, resolution.y);
}

std::vector<float> Renderer::normalize_vertex(MeshComponent &mesh) {
    ASSERT(mesh.vertices.size() % 3 == 0, "Invalid Vertex Length");

    int i = 0;
    while (i != mesh.vertices.size()) {
        mesh.vertices[i++] = (2.0f * mesh.vertices[i]) / resolution.x - 1.0f; // x
        mesh.vertices[i++] = 1.0f - (2.0f * mesh.vertices[i]) / resolution.y; // y
        mesh.vertices[i++] = mesh.vertices[i];                                // z
    }

    mesh.normalized = true;
    return mesh.vertices;
}

void Renderer::render_mesh(MeshComponent &mesh) {
    // send data to the gpu
    glBindBuffer(GL_ARRAY_BUFFER, mesh.VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * mesh.vertices.size(), mesh.vertices.data(), GL_STREAM_DRAW);

    glBindVertexArray(mesh.VAO);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * mesh.indices.size(), mesh.indices.data(), GL_STREAM_DRAW);

    glUseProgram(mesh.program);
    glUniform1ui(mesh.glu_time, time);
    glUniform2f(mesh.glu_resolution, resolution.x, resolution.y);

    // draw object on screen
    glDrawElements(GL_TRIANGLES, mesh.indices.size(), GL_UNSIGNED_INT, 0);

    // cleanup
    glBindVertexArray(0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}
