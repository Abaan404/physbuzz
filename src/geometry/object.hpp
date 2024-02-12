#pragma once

#include "../renderer/texture.hpp"
#include <memory>

enum class Objects {
    Unknown = -1,
    Box,
    Circle,
    Triangle,
    Bezier,
    Polygon,
};

struct DynamicProperties {
    float mass = 0.0f;
    float intertia = 0.0f;
    glm::vec2 velocity = glm::vec2(0.0f, 0.0f);
    glm::vec2 acceleration = glm::vec2(0.0f, 0.0f);
};

struct AABB {
    float x;
    float y;
    float w;
    float h;
};

class GameObject {
  public:
    GameObject(Objects identifier, glm::vec2 position);

    std::unique_ptr<TextureObject> texture;

    glm::vec2 position;
    float rotation;
    AABB rect;

    DynamicProperties dynamics;
    Objects identifier = Objects::Unknown;
};
