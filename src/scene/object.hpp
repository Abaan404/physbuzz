#pragma once

#include "component.hpp"
#include "defines.hpp"
#include <vector>

class Object {
  public:
    Object(ComponentManager &component_manager, ObjectID id);
    Object(const Object &other);
    Object &operator=(const Object &other);

    template <typename T>
    T &get_component() {
        return component_manager.get_component<T>(id);
    }

    template <typename T>
    void add_component(T &component) {
        component_manager.add_component<T>(component, id);
    }

    template <typename T>
    void remove_component() {
        component_manager.remove_component<T>(id);
    }

    template <typename T>
    bool has_component() {
        return component_manager.has_component<T>(id);
    }

    ObjectID id;

  private:
    ComponentManager &component_manager;
};

class ObjectManager {
  public:
    Object &create_object(ComponentManager &component_manager);
    void delete_object(ComponentManager &component_manager, ObjectID id);

    Object &get_object(ObjectID id);
    std::vector<Object> &get_objects();

  private:
    ObjectID object_counter = 0;
    std::vector<Object> objects;
    std::unordered_map<ObjectID, std::size_t> objectid_map;
};
