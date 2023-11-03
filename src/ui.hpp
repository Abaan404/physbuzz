#pragma once

#include "painter.hpp"

enum class InterfaceType {
    Debug,
    Info,
    MaxValue // not an actual type, just used to loop back to idx 0
};

class UserInferface {
  public:
    UserInferface(Painter &painter);

    void draw();

    InterfaceType interface_type;

  private:
    Painter &painter;
};
