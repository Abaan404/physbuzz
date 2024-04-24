#include "scene.hpp"

Object &Scene::create_object() {
    return obj_manager.create_object(comp_manager);
}

void Scene::delete_object(ObjectID id) {
    obj_manager.delete_object(comp_manager, id);
}

Object &Scene::get_object(ObjectID id) {
    return obj_manager.get_object(id);
}
