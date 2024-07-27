#include "renderer.hpp"

#include "window.hpp"
#include <GLFW/glfw3.h>
#include <glad/gl.h>

namespace Physbuzz {

RenderComponent::RenderComponent(const Mesh &mesh, const ShaderPipeline &pipeline, const Texture &texture)
    : mesh(mesh),
      pipeline(pipeline),
      texture(texture) {}

RenderComponent::~RenderComponent() {}

// NOTE: how should each member be bound to a render component if its shared
// across multiple RenderComponents? A tight coupling between a ResourceManager
// may be required. Or an ID setup

void RenderComponent::build() {
    mesh.build();
    pipeline.build();
}

void RenderComponent::destroy() {
    pipeline.destroy();
    mesh.destroy();
}

void RenderComponent::bind() const {
    pipeline.bind();
    texture.bind();
    mesh.bind();
}

void RenderComponent::draw() const {
    mesh.draw();
}

void RenderComponent::unbind() const {
    mesh.unbind();
    texture.unbind();
    pipeline.unbind();
}

Renderer::Renderer(Window &window)
    : m_Window(window) {}

Renderer::Renderer(const Renderer &other) : m_Window(other.m_Window) {
    if (this != &other) {
        copy(other);
    }
}

Renderer &Renderer::operator=(const Renderer &other) {
    if (this != &other) {
        copy(other);
    }

    return *this;
}

Renderer::~Renderer() {}

void Renderer::copy(const Renderer &other) {
    target(other.m_Framebuffer);

    m_Window = other.m_Window;
    m_Resolution = other.m_Resolution;
}

void Renderer::build() {
    target(nullptr);
}

void Renderer::destroy() {}

const Window &Renderer::getWindow() const {
    return m_Window;
}

const glm::ivec2 &Renderer::getResolution() const {
    return m_Resolution;
}

void Renderer::target(Framebuffer *framebuffer) {
    m_Framebuffer = framebuffer;

    if (framebuffer) {
        framebuffer->bind();
        resize(framebuffer->getResolution());
    } else {
        framebuffer->unbind();
        resize(m_Window.getResolution());
    }
}

void Renderer::clear(const glm::vec4 &color) {
    m_Framebuffer->clear(color);
}

void Renderer::resize(const glm::ivec2 &resolution) {
    if (m_Framebuffer == nullptr) {
        m_Window.setResolution(resolution);
    } else {
        m_Framebuffer->resize(resolution);
    }

    m_Resolution = resolution;
    glViewport(0, 0, resolution.x, resolution.y);
}

void Renderer::normalize(Mesh &mesh) {
    mesh.isScaled = true;
    for (std::size_t i = 0; i < mesh.vertices.size(); i++) {
        mesh.vertices[i].position.x = (2.0f * mesh.positions[i].x) / m_Resolution.x - 1.0f;
        mesh.vertices[i].position.y = 1.0f - (2.0f * mesh.positions[i].y) / m_Resolution.y;
    }
}

// way too many draw calls i know
void Renderer::render(RenderComponent &render) {
    // TODO skip requiring a normalize step using a projection
    if (!render.mesh.isScaled) {
        normalize(render.mesh);
    }

    render.bind();
    render.draw();
    render.unbind();
}

} // namespace Physbuzz
