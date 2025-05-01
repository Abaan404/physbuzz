#pragma once

#include <physbuzz/ecs/scene.hpp>
#include <physbuzz/misc/clock.hpp>
#include <physbuzz/render/camera.hpp>
#include <physbuzz/render/framebuffer.hpp>
#include <physbuzz/render/model.hpp>
#include <physbuzz/window/window.hpp>

class Renderer : public Physbuzz::System<Physbuzz::ModelComponent, Physbuzz::TransformComponent> {
  public:
    Renderer(Physbuzz::Window *window);
    ~Renderer();

    void build() override;
    void destroy() override;

    void target(Physbuzz::Framebuffer *framebuffer);

    void clear(const glm::vec4 &color);
    void resize(const glm::ivec2 &resolution);

    void tick(Physbuzz::Scene &scene);
    void render(Physbuzz::Scene &scene, Physbuzz::ObjectID id);

    const Physbuzz::Clock &getClock() const;
    const glm::ivec2 &getResolution() const;

    Physbuzz::Camera *activeCamera = nullptr;

  private:
    Physbuzz::Clock m_Clock;

    glm::ivec2 m_Resolution;
    glm::vec4 m_ClearColor = glm::vec4(0.0f, 0.0f, 0.0f, 0.0f);

    // FIXME: target only FBOs, remove dependency on a "window"
    Physbuzz::Framebuffer *m_Framebuffer = nullptr;
    Physbuzz::Window *m_Window = nullptr;
};
