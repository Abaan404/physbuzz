#pragma once

#include "objects/common.hpp"
#include <physbuzz/physics/dynamics.hpp>
#include <physbuzz/render/camera.hpp>
#include <physbuzz/render/framebuffer.hpp>
#include <physbuzz/render/mesh.hpp>
#include <physbuzz/resources/manager.hpp>
#include <physbuzz/window/window.hpp>

class Renderer : public Physbuzz::System<Physbuzz::MeshComponent, ResourceIdentifierComponent> {
  public:
    Renderer(Physbuzz::Window *window, Physbuzz::ResourceManager *resources);
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
    Physbuzz::ResourceManager *m_Resources = nullptr;
};
