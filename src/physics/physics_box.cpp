#include "physics_box.hpp"
#include <stdexcept>

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

bool PhysicsBox::collides(PhysicsObject *object) {
    throw std::runtime_error("Undefined Object Collision");
}
