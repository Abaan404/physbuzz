#pragma once

#include "ui.hpp"
#include <memory>
#include <physbuzz/window/window.hpp>
#include <unordered_map>

class InterfaceManager : public Physbuzz::System<> {
  public:
    struct Info {
        Physbuzz::Window *window;
    };

    InterfaceManager(const Info &info);

    bool build() override;
    bool destroy() override;
    void tick();

    bool draw = false;

  private:
    Info m_Info;

    std::unordered_map<std::string, std::shared_ptr<IUserInterface>> m_Interfaces;
};
