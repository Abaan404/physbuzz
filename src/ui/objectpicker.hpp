#pragma once

#include "ui.hpp"
#include <imgui.h>
#include <physbuzz/ecs/scene.hpp>
#include <physbuzz/render/camera.hpp>
#include <physbuzz/render/framebuffer.hpp>

class ObjectPicker : public IUserInterface {
  public:
    ObjectPicker(Physbuzz::Scene *scene);
    ~ObjectPicker();

    void draw() override;

  private:
    Physbuzz::Scene m_PickerScene;
    ImVec2 m_PreviewSize = {120, 120};
};
