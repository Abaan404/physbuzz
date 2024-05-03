#pragma once

#include "debug.hpp"
#include "defines.hpp"
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace Physbuzz {

class IComponentArray {
  public:
    virtual ~IComponentArray() = default;
    virtual void objectDestroyed(ObjectID object) = 0;
};

template <typename T>
class ComponentArray : public IComponentArray {
  public:
    void addComponent(T &component, ObjectID id) {
        ASSERT(m_ObjectIdMap.find(id) == m_ObjectIdMap.end(), "Component Already Exists.");

        // simply append to the end of the array
        m_ObjectIdMap[id] = m_Components.size();
        m_Components.emplace_back(component);
        m_PrevId = id;
    }

    void removeComponent(ObjectID id) {
        ASSERT(m_ObjectIdMap.find(id) != m_ObjectIdMap.end(), "Component Does Not Exist.");
        ASSERT(m_Components.size() > 0, "Removing From An Empty ComponentArray.");

        // get the component idx from the object
        std::size_t idx = m_ObjectIdMap[id];
        m_ObjectIdMap.erase(id);

        // swap elements and update map (if the idx isnt the end)
        if (idx != m_Components.size() - 1) {
            std::iter_swap(m_Components.begin() + idx, m_Components.end() - 1);
            m_ObjectIdMap[m_PrevId] = idx;
        }

        // pop component
        m_Components.pop_back();
    }

    T &getComponent(ObjectID id) {
        ASSERT(m_ObjectIdMap.find(id) != m_ObjectIdMap.end(), "Component Does Not Exist.");

        return m_Components[m_ObjectIdMap[id]];
    }

    std::vector<T> &getComponents() {
        return m_Components;
    }

    bool hasComponent(ObjectID id) {
        return m_ObjectIdMap.find(id) != m_ObjectIdMap.end();
    }

    void objectDestroyed(ObjectID id) override {
        if (hasComponent(id)) {
            removeComponent(id);
        }
    }

  private:
    std::vector<T> m_Components;
    std::unordered_map<ObjectID, std::size_t> m_ObjectIdMap;

    ObjectID m_PrevId = -1;
};

class ComponentManager {
  public:
    template <typename T>
    void addComponent(T &component, ObjectID object) {
        getComponentArray<T>()->addComponent(component, object);
    }

    template <typename T>
    void removeComponent(ObjectID object) {
        getComponentArray<T>()->removeComponent(object);
    }

    template <typename T>
    T &getComponent(ObjectID object) {
        return getComponentArray<T>()->getComponent(object);
    }

    template <typename T>
    std::vector<T> &getComponents() {
        return getComponentArray<T>()->getComponents();
    }

    template <typename T>
    bool hasComponent(ObjectID id) {
        return getComponentArray<T>()->hasComponent(id);
    }

    void objectDestroyed(ObjectID id) {
        for (const auto &pair : components) {
            pair.second->objectDestroyed(id);
        }
    }

  private:
    template <typename T>
    std::shared_ptr<ComponentArray<T>> getComponentArray() {
        std::string name = typeid(T).name();

        if (components.find(name) == components.end()) {
            components[name] = std::make_shared<ComponentArray<T>>();
        }

        return std::static_pointer_cast<ComponentArray<T>>(components[name]);
    }

    std::unordered_map<std::string, std::shared_ptr<IComponentArray>> components;
};

} // namespace Physbuzz
