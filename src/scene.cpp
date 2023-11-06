#include "scene.hpp"

void SceneManager::tick() {
    std::shared_ptr<GameObject> object;
    for (auto obj1 = objects.begin(); obj1 != objects.end(); obj1++) {
        object = *obj1;

        // apply dynamics
        dynamics.tick_object(object);

        // collision check
        std::shared_ptr<GameObject> other;
        for (auto obj2 = objects.begin(); obj2 != objects.end(); obj2++) {
            other = *obj2;

            // ignore if obj1 is obj2
            if (object == other) continue;

            collision.collides(object, other);
        }
    }
}
