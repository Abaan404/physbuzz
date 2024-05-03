#include "scene.hpp"

namespace Physbuzz {

Object &Scene::createObject() {
    return objectManager.createObject(componentManager);
}

void Scene::deleteObject(ObjectID id) {
    objectManager.deleteObject(componentManager, id);
}

Object &Scene::getObject(ObjectID id) {
    return objectManager.getObject(id);
}

std::vector<Object> &Scene::getObjects() {
    return objectManager.getObjects();
}

} // namespace Physbuzz
