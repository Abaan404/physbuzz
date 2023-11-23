#include "dynamics.hpp"

void Dynamics::tick_object(std::shared_ptr<GameObject> object) {
    // ignore non physics objects
    // eh not the most ideal solution but it works, idc
    switch (object->identifier) {
    case Objects::PhysicsBox:
        apply_dynamics(*std::static_pointer_cast<PhysicsBox>(object));
        break;

    case Objects::PhysicsCircle:
        apply_dynamics(*std::static_pointer_cast<PhysicsCircle>(object));
        break;

    default:
        return;
    }
}

void Dynamics::apply_dynamics(PhysicsBox &box) {
}

void Dynamics::apply_dynamics(PhysicsCircle &circle) {
    circle.position += circle.dynamics.velocity + 0.5f * circle.dynamics.acceleration;
    circle.rect = {circle.position.x - circle.radius, circle.position.y - circle.radius, 2 * circle.radius, 2 * circle.radius};
}
