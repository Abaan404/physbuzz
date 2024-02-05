#include "circle.hpp"

Circle::Circle(glm::vec2 position, float radius, float mass) : GameObject(Objects::Circle, position), radius(radius) {
    GameObject::rect = {position.x - radius, position.y - radius, 2 * radius, 2 * radius};
    GameObject::dynamics.mass = mass;
}
