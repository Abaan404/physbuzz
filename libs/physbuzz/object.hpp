#pragma once

#include "component.hpp"
#include "defines.hpp"
#include <vector>

namespace Physbuzz {

class Object {
  public:
    Object(ComponentManager &component_manager, ObjectID id);
    Object(const Object &other);
    Object &operator=(const Object &other);

    template <typename T>
    T &getComponent() {
        return m_ComponentManager.getComponent<T>(m_Id);
    }

    template <typename T>
    void addComponent(T component) {
        m_ComponentManager.addComponent<T>(component, m_Id);
    }

    template <typename T>
    void removeComponent() {
        m_ComponentManager.removeComponent<T>(m_Id);
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
    Object &createObject(ComponentManager &component_manager);
    void deleteObject(ComponentManager &component_manager, ObjectID id);

    Object &getObject(ObjectID id);
    std::vector<Object> &getObjects();

  private:
    ObjectID m_ObjectCounter = 0;
    std::vector<Object> m_Objects;
    std::unordered_map<ObjectID, std::size_t> m_ObjectIdMap;
};

}
