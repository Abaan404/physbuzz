#pragma once

#include "../debug.hpp"
#include "defines.hpp"
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

class IComponentArray {
  public:
    virtual ~IComponentArray() = default;
    virtual void object_destroyed(ObjectID object) = 0;
};

template <typename T>
class ComponentArray : public IComponentArray {
  public:
    void add_component(T &component, ObjectID id) {
        ASSERT(objectid_map.find(id) == objectid_map.end(), "Component Already Exists.");

        // simply append to the end of the array
        objectid_map[id] = components.size();
        components.emplace_back(component);
        prev_id = id;
    }

    void remove_component(ObjectID id) {
        ASSERT(objectid_map.find(id) != objectid_map.end(), "Component Does Not Exist.");
        ASSERT(components.size() > 0, "Removing From An Empty ComponentArray.");

        // get the component idx from the object
        std::size_t idx = objectid_map[id];
        objectid_map.erase(id);

        // swap and erase component
        std::iter_swap(components.begin() + idx, components.end());
        components.erase(components.end());

        // update id map
        objectid_map[prev_id] = idx;
    }

    T &get_component(ObjectID id) {
        ASSERT(objectid_map.find(id) != objectid_map.end(), "Component Does Not Exist.");

        return components[objectid_map[id]];
    }

    std::vector<T> &get_components() {
        return components;
    }

    void object_destroyed(ObjectID id) override {
        remove_component(id);
    }

  private:
    std::vector<T> components;
    std::unordered_map<ObjectID, std::size_t> objectid_map;

    ObjectID prev_id = -1;
};

class ComponentManager {
  public:
    template <typename T>
    void add_component(T &component, ObjectID object) {
        get_component_array<T>()->add_component(component, object);
    }

    template <typename T>
    void remove_component(ObjectID object) {
        get_component_array<T>()->remove_component(object);
    }

    template <typename T>
    T &get_component(ObjectID object) {
        return get_component_array<T>()->get_component(object);
    }

    template <typename T>
    std::vector<T> &get_components() {
        return get_component_array<T>()->get_components();
    }

    template <typename T>
    bool has_component() {
        std::string name = typeid(T).name();
        return components.find(name) != components.end();
    }

    void object_destroyed(ObjectID id) {
        for (const auto &pair : components) {
            pair.second->object_destroyed(id);
        }
    }

  private:
    template <typename T>
    std::shared_ptr<ComponentArray<T>> get_component_array() {
        std::string name = typeid(T).name();

        if (components.find(name) == components.end()) {
            components[name] = std::make_shared<ComponentArray<T>>();
        }

        return std::static_pointer_cast<ComponentArray<T>>(components[name]);
    }

    std::unordered_map<std::string, std::shared_ptr<IComponentArray>> components;
};
