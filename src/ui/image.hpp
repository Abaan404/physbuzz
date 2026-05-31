#pragma once

#include "ui.hpp"
#include <imgui.h>
#include <physbuzz/graphics/memory.hpp>
#include <physbuzz/resources/resource.hpp>

template <typename T>
class Image : public IUserInterface {
  public:
    Image(const Physbuzz::Resource<T> &resource, Physbuzz::Scene *scene);
    void draw() override;

  private:
    ImVec2 getResolution();

    Physbuzz::Resource<T> m_Resource;
    Physbuzz::Image::ViewInfo m_ViewInfo = {
        .type = Physbuzz::Image::ViewType::e2D,
        .subresourceRange = {
            .aspectMask = Physbuzz::Image::AspectFlags::eColor,
            .levelCount = 1,
            .layerCount = 1,
        },
    };

    float m_WidgetHeight = 0.0f;
};
