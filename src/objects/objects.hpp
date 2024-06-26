#pragma once

#include <functional>
#include <physbuzz/object.hpp>
#include <physbuzz/scene.hpp>

template <typename T>
    requires std::is_aggregate_v<T>
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
    Line,
    Unknown,
};

struct IdentifiableComponent {
    ObjectType type = ObjectType::Unknown;
    std::string name = "Unknown";
    bool hidden = false;
};

struct RebuildableComponent {
    std::function<void(Physbuzz::Object&)> rebuild;
};
