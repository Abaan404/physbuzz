#include "dynamics.hpp"

#include <glm/glm.hpp>

void Dynamics::tick(std::shared_ptr<GameObject> object) {
    // ignore non physics objects
    // eh not the most ideal solution but it works, idc
    switch (object->identifier) {
    case Objects::Box:
        apply_dynamics(*std::static_pointer_cast<Box>(object));
        break;

    case Objects::Circle:
        apply_dynamics(*std::static_pointer_cast<Circle>(object));
        break;

    default:
        return;
    }
}

void Dynamics::apply_dynamics(Box &box) {
}

void Dynamics::apply_dynamics(Circle &circle) {
    circle.position += circle.dynamics.velocity + 0.5f * circle.dynamics.acceleration;
    circle.rect = {circle.position.x - circle.radius, circle.position.y - circle.radius, 2 * circle.radius, 2 * circle.radius};
}
