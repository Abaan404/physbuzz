#pragma once

#include "component.hpp"
#include "defines.hpp"
#include "object.hpp"

class Scene {
  public:
    Object &create_object();
    void delete_object(ObjectID id);
    Object &get_object(ObjectID id);

    template <typename T>
    std::vector<T> &get_components() {
        return comp_manager.get_components<T>();
    }

    std::vector<Object> &get_objects() {
        return obj_manager.get_objects();
    }

  private:
    ComponentManager comp_manager;
    ObjectManager obj_manager;
};
