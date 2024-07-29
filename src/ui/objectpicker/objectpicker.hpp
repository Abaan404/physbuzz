#pragma once

#include "../ui.hpp"
#include <imgui.h>
#include <physbuzz/camera.hpp>
#include <physbuzz/framebuffer.hpp>
#include <physbuzz/scene.hpp>

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
