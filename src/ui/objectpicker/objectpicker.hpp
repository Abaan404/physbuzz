#pragma once

#include "../ui.hpp"
#include <imgui.h>
#include <physbuzz/ecs/scene.hpp>
#include <physbuzz/render/camera.hpp>
#include <physbuzz/render/framebuffer.hpp>

struct PickableComponent {
    bool selected = false;
    Physbuzz::Framebuffer framebuffer;
};

class ObjectPicker : public IUserInterface {
  public:
    ObjectPicker();
    ~ObjectPicker();

    void draw() override;

  private:
    Physbuzz::Scene m_Scene;
    Physbuzz::Camera m_Camera;
    ImVec2 m_PreviewSize = {120, 120};
};
