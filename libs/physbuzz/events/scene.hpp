#pragma once

#include "physbuzz/defines.hpp"

namespace Physbuzz {

class Scene;

struct OnSceneClear {
    Scene *scene;
};

struct OnObjectCreateEvent {
    Scene *scene;

    ObjectID id;
};

struct OnObjectDeleteEvent {
    Scene *scene;

    ObjectID id;
};

} // namespace Physbuzz
