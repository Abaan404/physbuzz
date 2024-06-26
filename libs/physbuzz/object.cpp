#include "object.hpp"

namespace Physbuzz {

Object::Object(ComponentManager &component_manager, ObjectID id) : m_ComponentManager(component_manager),
                                                                   m_Id(id) {}

Object::Object(const Object &other) : m_ComponentManager(other.m_ComponentManager),
                                      m_Id(other.m_Id) {}

Object &Object::operator=(const Object &other) {
    m_ComponentManager = other.m_ComponentManager;
    m_Id = other.m_Id;

    return *this;
}

void Object::eraseComponents() {
    m_ComponentManager.objectDestroyed(m_Id);
}

ObjectID Object::getId() const {
    return m_Id;
}

Object &ObjectManager::getObject(ObjectID id) {
    return m_Map.get(id);
}

ObjectID ObjectManager::createObject(ComponentManager &componentManager) {
    return createObject(componentManager, m_ObjectCounter++);
}

ObjectID ObjectManager::createObject(ComponentManager &componentManager, ObjectID id) {
    deleteObject(componentManager, id);

    Object object = Object(componentManager, id);
    return m_Map.insert(object, id);
}

bool ObjectManager::deleteObject(ComponentManager &componentManager, ObjectID id) {
    // if remove was successful, remove related components
    if (m_Map.remove(id)) {
        componentManager.objectDestroyed(id);
        return true;
    }

    return false;
}

bool ObjectManager::hasObject(ObjectID id) const {
    return m_Map.contains(id);
}

std::vector<Object> &ObjectManager::getObjects() {
    return m_Map.getArray();
}

void ObjectManager::clearObjects() {
    for (auto &object : getObjects()) {
        object.eraseComponents();
    }

    m_Map.clear();
    m_ObjectCounter = 0;
}

} // namespace Physbuzz
