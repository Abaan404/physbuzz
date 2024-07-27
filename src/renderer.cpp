#include "renderer.hpp"

Renderer::Renderer(Physbuzz::Window &window)
    : m_Renderer(window) {}

Renderer::~Renderer() {}

void Renderer::build() {
    m_Renderer.build();
}

void Renderer::destroy() {
    m_Renderer.destroy();
}

void Renderer::render(Physbuzz::Scene &scene) {
    m_Clock.tick();
    m_Renderer.clear(m_ClearColor);

    for (auto &component : scene.getComponents<Physbuzz::RenderComponent>()) {
        render(component);
    }
}

void Renderer::render(Physbuzz::RenderComponent &render) {
    // bind will be called twice, needs to be fixed in library code
    render.pipeline.bind();
    render.pipeline.setUniform("u_Time", static_cast<unsigned int>(m_Clock.getTime()));
    render.pipeline.setUniform("u_TimeDelta", static_cast<unsigned int>(m_Clock.getDelta()));
    render.pipeline.setUniform("u_Resolution", m_Renderer.getResolution());
    render.pipeline.setUniform("u_Texture", render.texture.getUnit());
    render.pipeline.unbind();

    m_Renderer.render(render);
}

const Physbuzz::Clock &Renderer::getClock() const {
    return m_Clock;
}

// TODO this should be a const reference
Physbuzz::Renderer &Renderer::getRenderer() {
    return m_Renderer;
}
