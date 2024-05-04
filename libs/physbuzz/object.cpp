#include "object.hpp"
#include "physbuzz/debug.hpp"

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

Object &ObjectManager::getObject(ObjectID id) {
    std::size_t idx = m_ObjectIdMap[id];
    return m_Objects[idx];
}

Object &ObjectManager::createObject(ComponentManager &component_manager) {
    // create an object
    m_Objects.emplace_back(component_manager, m_ObjectCounter);
    m_ObjectIdMap[m_Objects.size()] = m_ObjectCounter++;

    return m_Objects.back();
}

void ObjectManager::deleteObject(ComponentManager &component_manager, ObjectID id) {
    ASSERT(m_ObjectIdMap.contains(id), "Object doesnt exist in map.")

    // pop id from map
    std::size_t idx = m_ObjectIdMap[id];
    m_ObjectIdMap.erase(id);

    // swap elements and update map (if the idx isnt the end)
    if (idx != m_Objects.size() - 1) {
        std::iter_swap(m_Objects.begin() + idx, m_Objects.end() - 1);
        m_ObjectIdMap[m_Objects[idx].getId()] = idx;
    }

    // pop object
    m_Objects.pop_back();

    // update components
    component_manager.objectDestroyed(id);
}

std::vector<Object> &ObjectManager::getObjects() {
    return m_Objects;
}

ObjectID Object::getId() const {
    return m_Id;
}

} // namespace Physbuzz
