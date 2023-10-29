#include "physics_circle.hpp"
#include "physics_box.hpp"
#include <stdexcept>

bool PhysicsBox::collides(GameObject *object) {
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

// ref: https://code.tutsplus.com/collision-detection-using-the-separating-axis-theorem--gamedev-169t
// currently just assumes every box is perpendicular for now
bool PhysicsBox::collides(PhysicsBox *box) {
    // if not colliding on any axis
    if (this->max[0] < box->min[0] or this->min[0] > box->max[0])
        return false;

    if (this->max[1] < box->min[1] or this->min[1] > box->max[1])
        return false;

    return true;
}

bool PhysicsBox::collides(PhysicsCircle *circle) {
    return false;
}
