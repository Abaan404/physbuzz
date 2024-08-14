#pragma once

#include "../ecs/component.hpp"
#include <memory>

namespace Physbuzz {

class Scene;

struct OnSceneClear {
    Scene *scene;
};

struct OnObjectCreateEvent {
    Scene *scene;
    ObjectID object;
};

struct OnObjectEraseEvent {
    Scene *scene;
    ObjectID object;
};

template <typename T>
struct OnComponentSetEvent {
    Scene *scene;
    ObjectID object;

    T *component;
};

template <typename T>
struct OnComponentEraseEvent {
    Scene *scene;
    ObjectID object;

    T *component;
};

template <typename T>
struct OnSystemEmplaceEvent {
    Scene *scene;

    std::shared_ptr<T> system;
};

template <typename T>
struct OnSystemEraseEvent {
    Scene *scene;

    std::shared_ptr<T> system;
};

} // namespace Physbuzz
