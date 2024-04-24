#include "object.hpp"

Object::Object(ComponentManager &component_manager, ObjectID id) : component_manager(component_manager),
                                                                   id(id) {}

Object::Object(const Object &other) : component_manager(other.component_manager),
                                      id(other.id) {}

Object &Object::operator=(const Object &other) {
    this->component_manager = other.component_manager;
    this->id = other.id;

    return *this;
}

Object &ObjectManager::get_object(ObjectID id) {
    std::size_t idx = objectid_map[id];
    return objects[idx];
}

Object &ObjectManager::create_object(ComponentManager &component_manager) {
    // create an object
    objects.emplace_back(component_manager, object_counter);
    objectid_map[objects.size()] = object_counter++;

    return objects.back();
}

void ObjectManager::delete_object(ComponentManager &component_manager, ObjectID id) {
    // pop id from map
    std::size_t idx = objectid_map[id];
    objectid_map.erase(id);

    // swap elements and erase
    std::iter_swap(objects.begin() + idx, objects.end());
    objects.erase(objects.end());

    // update id map
    objectid_map[objects[idx].id] = idx;

    // update components
    component_manager.object_destroyed(id);
}

std::vector<Object> &ObjectManager::get_objects() {
    return objects;
}
