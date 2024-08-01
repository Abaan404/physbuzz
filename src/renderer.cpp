#include "renderer.hpp"
#include "physbuzz/dynamics.hpp"
#include "physbuzz/logging.hpp"

#include <glm/ext/matrix_clip_space.hpp>

Renderer::Renderer(Physbuzz::Window *window)
    : renderer(window) {}

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

    for (auto &object : scene.getObjects()) {

        if (!object.hasComponent<Physbuzz::RenderComponent>() || !object.hasComponent<Physbuzz::TransformableComponent>()) {
            continue;
        }

        render(object);
    }
}

void Renderer::render(Physbuzz::Object &object) const {
    Physbuzz::Logger::ASSERT(activeCamera != nullptr, "No camera bound to renderer");

    // bind will be called twice, needs to be fixed in library code
    const Physbuzz::RenderComponent render = object.getComponent<Physbuzz::RenderComponent>();
    render.pipeline.bind();

    // Time functions
    render.pipeline.setUniform<unsigned int>("u_Time", m_Clock.getTime());
    render.pipeline.setUniform<unsigned int>("u_TimeDelta", m_Clock.getDelta());

    // render info
    render.pipeline.setUniform("u_Resolution", renderer.getResolution());

    // bound texture
    render.pipeline.setUniform("u_Texture", render.texture.getUnit());

    // MVP camera info
    render.pipeline.setUniform("u_Model", object.getComponent<Physbuzz::TransformableComponent>().generateModel());
    render.pipeline.setUniform("u_View", activeCamera->getView());
    render.pipeline.setUniform("u_Projection", activeCamera->getProjection());

    render.pipeline.unbind();

    renderer.render(render);
}

const Physbuzz::Clock &Renderer::getClock() const {
    return m_Clock;
}
