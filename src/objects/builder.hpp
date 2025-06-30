#pragma once

#include <physbuzz/ecs/scene.hpp>

struct Buildable {};

template <typename T>
concept BuildableType = std::derived_from<T, Buildable>;

class ObjectBuilder {
  public:
    template <BuildableType T>
    static Physbuzz::ObjectID create(Physbuzz::Scene &scene, T &info) {
        Physbuzz::ObjectID id = scene.createObject();
        return create(scene, id, info);
    }

    template <BuildableType T>
    static Physbuzz::ObjectID create(Physbuzz::Scene &scene, Physbuzz::ObjectID id, T &info);
};
