#include "scene.hpp"

namespace Physbuzz {

ObjectID Scene::createObject() {
    return m_ObjectManager.createObject(m_ComponentManager);
}

ObjectID Scene::createObject(ObjectID id) {
    return m_ObjectManager.createObject(m_ComponentManager, id);
}

bool Scene::deleteObject(ObjectID id) {
    return m_ObjectManager.deleteObject(m_ComponentManager, id);
}

bool Scene::hasObject(ObjectID id) const {
    return m_ObjectManager.hasObject(id);
}

Object &Scene::getObject(ObjectID id) {
    return m_ObjectManager.getObject(id);
}

std::vector<Object> &Scene::getObjects() {
    return m_ObjectManager.getObjects();
}

} // namespace Physbuzz
