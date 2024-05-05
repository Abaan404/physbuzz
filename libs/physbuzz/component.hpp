#pragma once

#include "containers/vectormap.hpp"
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
    void setComponent(T &component, ObjectID id) {
        m_Map.insert(component, id);
    }

    bool removeComponent(ObjectID id) {
        return m_Map.remove(id);
    }

    bool hasComponent(ObjectID id) {
        return m_Map.contains(id);
    }

    T &getComponent(ObjectID id) {
        ASSERT(m_Map.contains(id), "Component Does Not Exist.");
        return m_Map.get(id);
    }

    std::vector<T> &getComponents() {
        return m_Map.getArray();
    }

    void objectDestroyed(ObjectID id) override {
        if (!m_Map.contains(id)) {
            return;
        }

        removeComponent(id);
    }

  private:
    VectorMap<ObjectID, T> m_Map;
};

class ComponentManager {
  public:
    // how any of these symbols are resolved is beyond me.
    template <typename T>
    void setComponent(T &component, ObjectID object) {
        getComponentArray<T>()->setComponent(component, object);
    }

    template <typename T>
    bool removeComponent(ObjectID object) {
        return getComponentArray<T>()->removeComponent(object);
    }

    template <typename T>
    bool hasComponent(ObjectID id) {
        return getComponentArray<T>()->hasComponent(id);
    }

    template <typename T>
    T &getComponent(ObjectID object) {
        return getComponentArray<T>()->getComponent(object);
    }

    template <typename T>
    std::vector<T> &getComponents() {
        return getComponentArray<T>()->getComponents();
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

        if (!components.contains(name)) {
            components[name] = std::make_shared<ComponentArray<T>>();
        }

        return std::static_pointer_cast<ComponentArray<T>>(components[name]);
    }

    std::unordered_map<std::string, std::shared_ptr<IComponentArray>> components;
};

} // namespace Physbuzz
