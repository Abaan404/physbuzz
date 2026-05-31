#pragma once

#include "ui.hpp"
#include "image.hpp"
#include <physbuzz/graphics/descriptors/attachment.hpp>

class RenderGraph : public IUserInterface {
  public:
    RenderGraph(Physbuzz::Scene *scene);

    void draw() override;

  private:
    std::unordered_map<Physbuzz::ResourceID, Image<Physbuzz::Attachment>> m_AttachmentWindows;

    bool m_EnableShadows = false;
    std::string m_SelectedRenderer = "Unknown";
};
