#include "collision.hpp"

bool Collision::check_collision(GameObject *object1, GameObject *object2) {
    switch (object1->identifier) {
    case Objects::PhysicsBox:
        switch (object2->identifier) {
        case Objects::PhysicsBox:
            return check_collision_box_box(static_cast<PhysicsBox *>(object1), static_cast<PhysicsBox *>(object2));

        case Objects::PhysicsCircle:
            return check_collision_box_circle(static_cast<PhysicsBox *>(object1), static_cast<PhysicsCircle *>(object2));

        default:
            break;
        }
        break;
    case Objects::PhysicsCircle:
        switch (object2->identifier) {
        case Objects::PhysicsCircle:
            return check_collision_circle_circle(static_cast<PhysicsCircle *>(object1), static_cast<PhysicsCircle *>(object2));

        case Objects::PhysicsBox:
            return check_collision_box_circle(static_cast<PhysicsBox *>(object2), static_cast<PhysicsCircle *>(object1));

        default:
            break;
        }
        break;
    default:
        break;
    }

    return false;
}

bool Collision::check_collision_box_box(PhysicsBox *object1, PhysicsBox *object2) {
    if (object1->max[0] < object2->min[0] or object1->min[0] > object2->max[0])
        return false;

    if (object1->max[1] < object2->min[1] or object1->min[1] > object2->max[1])
        return false;

    return true;
}

bool Collision::check_collision_circle_circle(PhysicsCircle *object1, PhysicsCircle *object2) {
    return false;
}

bool Collision::check_collision_box_circle(PhysicsBox *object1, PhysicsCircle *object2) {
    return false;
}
