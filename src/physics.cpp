#include "physics.hpp"
#include "geometry/object.hpp"
#include "physics/physics_box.hpp"
#include <cstdio>

void PhysicsContext::tick() {
    GameObject *object1, *object2;
    for (auto obj1 = objects->begin(); obj1 != objects->end(); obj1++) {
        object1 = *obj1;

        for (auto obj2 = objects->begin(); obj2 != objects->end(); obj2++) {
            object2 = *obj2;

            if (object1 == object2) continue;
            if (object1->collides(object2))
                printf("Collision Detected\n");
        }
    }
}
