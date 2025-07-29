#pragma once

#include "ui.hpp"

class Dockspace : public IUserInterface {
  public:
    Dockspace(Physbuzz::Scene *scene);
    ~Dockspace();

    void draw() override;

  private:
    bool m_Docked = false;
};
