#pragma once

#include "ui.hpp"
#include <memory>
#include <physbuzz/window/window.hpp>
#include <unordered_map>

class InterfaceManager : public Physbuzz::System<> {
  public:
    InterfaceManager();

    bool build() override;
    bool destroy() override;

    void tick();

    bool draw = false;

  private:
    std::unordered_map<std::string, std::shared_ptr<IUserInterface>> m_Interfaces;
};
