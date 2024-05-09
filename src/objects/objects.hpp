#pragma once

#include <physbuzz/object.hpp>
#include <physbuzz/scene.hpp>

template <typename T>
struct ObjectBuilder {
    static Physbuzz::ObjectID build(Physbuzz::Scene &scene, Physbuzz::ObjectID id, T &info) {
        scene.createObject(id);
        return ObjectBuilder<T>::build(scene.getObject(id), info);
    }

    static Physbuzz::ObjectID build(Physbuzz::Scene &scene, T &info) {
        Physbuzz::ObjectID id = scene.createObject();
        return ObjectBuilder<T>::build(scene.getObject(id), info);
    }

    static Physbuzz::ObjectID build(Physbuzz::Object &object, T &info);
};

enum class ObjectType {
    Circle,
    Quad,
    Unknown,
};

struct IdentifiableComponent {
    ObjectType type = ObjectType::Unknown;
    std::string name = "Unknown";
    bool hidden = false;
};
