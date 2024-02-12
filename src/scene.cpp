#include "scene.hpp"

void SceneManager::tick() {
    for (auto object : objects) {
        if (object->dynamics == nullptr)
            continue;

        object->dynamics->tick();

        for (auto other : objects) {
            if (object == other)
                continue;

            collision.tick(object, other);
        }
    }
}
