#include "physics_box.hpp"

PhysicsBox::PhysicsBox(glm::vec2 position, float width, float height, float mass) : Box(position, width, height) {
    GameObject::identifier = Objects::PhysicsBox;
    GameObject::dynamics.mass = mass;
}
