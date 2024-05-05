#pragma once

#include "component.hpp"
#include "defines.hpp"
#include "physbuzz/containers/vectormap.hpp"
#include <vector>

namespace Physbuzz {

class Object {
  public:
    Object(ComponentManager &component_manager, ObjectID id);
    Object(const Object &other);
    Object &operator=(const Object &other);

    template <typename T>
    void setComponent(T component) {
        m_ComponentManager.setComponent<T>(component, m_Id);
    }

    template <typename T>
    bool removeComponent() {
        return m_ComponentManager.removeComponent<T>(m_Id);
    }

    template <typename T>
    T &getComponent() {
        return m_ComponentManager.getComponent<T>(m_Id);
    }

    template <typename T>
    bool hasComponent() {
        return m_ComponentManager.hasComponent<T>(m_Id);
    }

    void eraseComponents();

    ObjectID getId() const;

  private:
    ComponentManager &m_ComponentManager;
    ObjectID m_Id;
};

class ObjectManager {
  public:
    ObjectID createObject(ComponentManager &componentManager);
    void deleteObject(ComponentManager &componentManager, ObjectID id);

    bool hasObject(ObjectID id);
    Object &getObject(ObjectID id);
    std::vector<Object> &getObjects();

  private:
    ObjectID m_ObjectCounter = 0;
    VectorMap<ObjectID, Object> m_Map;
};

} // namespace Physbuzz
