#include "physics_circle.hpp"

PhysicsCircle::PhysicsCircle(glm::vec2 position, float radius, float mass) : Circle(position, radius) {
    GameObject::identifier = Objects::PhysicsCircle;
    GameObject::dynamics.mass = mass;
}
