#include "renderer.hpp"

#include <glm/ext/matrix_clip_space.hpp>

Renderer::Renderer(Physbuzz::Window &window, Physbuzz::Camera &camera)
    : renderer(window), m_ActiveCamera(&camera) {}

Renderer::~Renderer() {}

void Renderer::build() {
    renderer.build();
}

void Renderer::destroy() {
    renderer.destroy();
}

void Renderer::render(Physbuzz::Scene &scene) {
    m_Clock.tick();
    renderer.clear(m_ClearColor);

    for (auto &component : scene.getComponents<Physbuzz::RenderComponent>()) {
        render(component);
    }
}

void Renderer::render(Physbuzz::RenderComponent &render) const {
    // bind will be called twice, needs to be fixed in library code
    render.pipeline.bind();

    // Time functions
    render.pipeline.setUniform<unsigned int>("u_Time", m_Clock.getTime());
    render.pipeline.setUniform<unsigned int>("u_TimeDelta", m_Clock.getDelta());

    // render info
    render.pipeline.setUniform("u_Resolution", renderer.getResolution());

    // bound texture
    render.pipeline.setUniform("u_Texture", render.texture.getUnit());

    // m_ActiveCamera.projection = glm::ortho(0.0f, static_cast<float>(1890), static_cast<float>(1007), 0.0f, -1.0f, 1.0f);

    // MVP camera info
    render.pipeline.setUniform("u_Model", m_ActiveCamera->model);
    render.pipeline.setUniform("u_View", m_ActiveCamera->view);
    render.pipeline.setUniform("u_Projection", m_ActiveCamera->projection);

    render.pipeline.unbind();

    renderer.render(render);
}

void Renderer::setCamera(const Physbuzz::Camera &camera) {
    m_ActiveCamera = &camera;
}

const Physbuzz::Clock &Renderer::getClock() const {
    return m_Clock;
}
