#pragma once

#include "ui.hpp"
#include <unordered_map>

class InterfaceManager {
  public:
    InterfaceManager(Physbuzz::Renderer &renderer);
    InterfaceManager(const InterfaceManager &other);
    InterfaceManager operator=(const InterfaceManager &other);
    ~InterfaceManager();

    void render();

    bool draw = false;
    std::unordered_map<std::string, std::shared_ptr<IUserInterface>> interfaces;

  private:
    Physbuzz::Renderer &m_Renderer;
};
