#include "physics.hpp"
#include <cstdio>

Physics::Physics(std::list<Object *> *objects) {
    this->objects = objects;
}

void Physics::tick() {
    // TODO like dont do this
    for (auto object = objects->begin(); object != objects->end(); object++) {
        Object *obj1 = *object;
        for (auto object = objects->begin(); object != objects->end(); object++) {
            Object *obj2 = *object;

            if (obj1 == obj2) continue;
            if (obj1->collides(obj2))
                printf("Collision Detected\n");
        }
    }
}
