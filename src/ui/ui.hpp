#pragma once

class IUserInterface {
  public:
    bool show = true;
    virtual void draw() = 0;
};
