#include "physics.hpp"

void PhysicsContext::tick() {
    GameObject *object1, *object2;
    for (auto obj1 = objects.begin(); obj1 != objects.end(); obj1++) {
        object1 = &**obj1;

        for (auto obj2 = objects.begin(); obj2 != objects.end(); obj2++) {
            object2 = &**obj2;

            // ignore if obj1 is obj2
            if (object1 == object2) continue;

            collision.check_collision(object1, object2);
        }
    }
}
