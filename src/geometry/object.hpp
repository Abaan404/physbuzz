#pragma once

#include "../renderer/texture.hpp"
#include "../dynamics/dynamics.hpp"
#include <memory>

enum class Objects {
    Unknown = -1,
    Box,
    Circle,
    Triangle,
    Bezier,
    Polygon,
};

class GameObject {
  public:
    GameObject(Objects identifier, glm::vec2 position);

    std::unique_ptr<TextureObject> texture;
    std::unique_ptr<DynamicObject> dynamics;

    glm::vec2 position;
    float rotation;

    Objects identifier = Objects::Unknown;
};
