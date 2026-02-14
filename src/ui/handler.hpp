#pragma once

#include "ui.hpp"
#include <memory>
#include <physbuzz/window/window.hpp>

class InterfaceManager : public Physbuzz::System<> {
  public:
    InterfaceManager();

    bool build() override;
    bool destroy() override;

    void tick();

    bool draw = false;

  private:
    std::vector<std::shared_ptr<IUserInterface>> m_Interfaces;
};
