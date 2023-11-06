#include "dynamics.hpp"

void Dynamics::tick_object(std::shared_ptr<GameObject> object) {
    // ignore non physics objects
    // eh not the most ideal solution but it works, idc
    switch (object->identifier) {
    case Objects::PhysicsBox:
    case Objects::PhysicsCircle:
        break;

    default:
        return;
    }

    // TODO like complete this stuff
}
