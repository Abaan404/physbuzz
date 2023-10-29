#include "physics_circle.hpp"
#include "physics_box.hpp"
#include <stdexcept>

bool PhysicsCircle::collides(GameObject *object) {
    switch (object->identifier) {
    case Objects::PhysicsBox:
        return this->collides((PhysicsBox *)object);
        break;

    case Objects::PhysicsCircle:
        return this->collides((PhysicsCircle *)object);
        break;

    case Objects::Unknown:
        throw std::runtime_error("Undefined Object Collision");

    case Objects::Box:
    case Objects::Circle:
        break;
    }
    return false;
}

bool PhysicsCircle::collides(PhysicsCircle *circle) {
    return false;
}

bool PhysicsCircle::collides(PhysicsBox *box) {
    return false;
}
