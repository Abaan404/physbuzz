#pragma once

#include <physbuzz/ecs/scene.hpp>
#include <type_traits>

template <typename T>
struct IsBuildable : std::false_type {};

template <typename T>
concept BuildableType =
    IsBuildable<T>::value;

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
