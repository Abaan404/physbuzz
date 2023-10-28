#include "physics.hpp"
#include "physics/physics_box.hpp"
#include <cstdio>

PhysicsContext::PhysicsContext(GameObjectStore *storage) : storage(storage) {}

void PhysicsContext::tick() {

    // TODO spend more time in making this easier to work with.
    // CollisionHandler Ideas: (Strategy Pattern moment?)
    // 1) Visitor/Double Dispatcher Patterns (dynamic cast is a bit iffy)
    // 2) Assign a collision-handling func ptrs to each object, and use a resolve func that uses an enum to infer physicsobject type
    // 3) Deal with each collision seperately (bruteforce)

    if (storage->phys_boxes.size() <= 0)
        return;

    long unsigned int i1;
    for (i1 = 0; i1 <= storage->phys_boxes.size(); i1++) {
        PhysicsBox box1 = storage->phys_boxes[i1];

        long unsigned int i2;
        for (i2 = 0; i2 <= storage->phys_boxes.size(); i2++) {
            PhysicsBox box2 = storage->phys_boxes[i2];

            if (i1 == i2) continue;
            if (box1.collides(&box2))
                printf("Collision Detected\n");
        }
    }

    // for (auto box1 = storage->phys_boxes.begin(); box1 != storage->phys_boxes.end(); box1++) {
    //     PhysicsBox obj1 = *box1;
    //
    //     for (auto box2 = storage->phys_boxes.begin(); box2 != storage->phys_boxes.end(); box2++) {
    //         PhysicsBox obj2 = *box2;
    //
    //         if (&obj1 == &obj2) continue;
    //         if (obj1.collides(&obj2))
    //             printf("Collision Detected\n");
    //         printf("%p %p\n", &obj1, &obj2);
    //     }
    // }
}
