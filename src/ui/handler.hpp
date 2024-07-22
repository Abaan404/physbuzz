#pragma once

#include <physbuzz/window.hpp>
#include "ui.hpp"
#include <memory>
#include <unordered_map>

class InterfaceManager {
  public:
    InterfaceManager();
    ~InterfaceManager();

    void build(const Physbuzz::Window &window);
    void destroy();
    void render();

    bool draw = false;

  private:
    std::unordered_map<std::string, std::shared_ptr<IUserInterface>> m_Interfaces;
};
