#include "physics_circle.hpp"
#include "physics_box.hpp"
#include <cstdio>
#include <stdexcept>

void PhysicsCircle::collides(GameObject *object) {
    switch (object->identifier) {
    case Objects::PhysicsBox:
        if (check_collision((PhysicsBox *)object))
            resolve_collision((PhysicsBox *)object);
        break;

    case Objects::PhysicsCircle:
        if (check_collision((PhysicsCircle *)object))
            resolve_collision((PhysicsCircle *)object);
        break;

    case Objects::Unknown:
        throw std::runtime_error("Undefined Object Collision");

    case Objects::Box:
    case Objects::Circle:
        break;
    }
}

bool PhysicsCircle::check_collision(PhysicsCircle *circle) {
    return false;
}

bool PhysicsCircle::check_collision(PhysicsBox *box) {
    return false;
}

void PhysicsCircle::resolve_collision(PhysicsCircle* circle) {
    printf("CircleCircle Collision Detected\n");
}

void PhysicsCircle::resolve_collision(PhysicsBox* box) {
    printf("CircleBox Collision Detected\n");
}
