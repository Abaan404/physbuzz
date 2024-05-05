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
    Object object = Object(componentManager, m_ObjectCounter);
    return m_Map.insert(object, m_ObjectCounter++);
}

void ObjectManager::deleteObject(ComponentManager &componentManager, ObjectID id) {
    m_Map.remove(id);

    // update components
    componentManager.objectDestroyed(id);
}

bool ObjectManager::hasObject(ObjectID id) {
    return m_Map.contains(id);
}

std::vector<Object> &ObjectManager::getObjects() {
    return m_Map.getArray();
}

} // namespace Physbuzz
