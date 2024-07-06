#pragma once

namespace Physbuzz {

class Object;

template <typename T>
struct OnComponentSetEvent {
    Object *object;

    T &component;
};

template <typename T>
struct OnComponentRemoveEvent {
    Object *object;

    T &component;
};

struct OnComponentEraseEvent {
    Object *object;
};

} // namespace Physbuzz
