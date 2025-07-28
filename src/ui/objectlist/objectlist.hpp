#pragma once

#include "../ui.hpp"

class ObjectList : public IUserInterface {
  public:
    ObjectList(Physbuzz::Scene *scene);

    void draw() override;
};
