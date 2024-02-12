#include "object.hpp"

#include <glad/gl.h>

GameObject::GameObject(Objects identifier, glm::vec2 position) : position(position), identifier(identifier) {
    texture = nullptr;
    dynamics = nullptr;
};
