#pragma once

#include "component.hpp"
#include "containers/contigiousmap.hpp"
#include "defines.hpp"

namespace Physbuzz {

class Object {
  public:
    Object(ComponentManager &component_manager, ObjectID id);
    Object(const Object &other);
    Object &operator=(const Object &other);

    template <typename T>
    void setComponent(T &component) {
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
    ObjectID createObject(ComponentManager &componentManager, ObjectID id);
    bool deleteObject(ComponentManager &componentManager, ObjectID id);
    bool hasObject(ObjectID id) const;
    void clearObjects();

    template <typename F>
        requires Comparator<F, Physbuzz::Object>
    void sortObjects(F comparator) {
        m_Map.sort(comparator);
    }

    Object &getObject(ObjectID id);
    std::vector<Object> &getObjects();

  private:
    ObjectID m_ObjectCounter = 0;
    ContiguousMap<ObjectID, Object> m_Map;
};

} // namespace Physbuzz
