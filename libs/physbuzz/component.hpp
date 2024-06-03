#pragma once

#include "containers/contigiousmap.hpp"
#include "defines.hpp"
#include "logging.hpp"
#include <concepts>
#include <memory>

namespace Physbuzz {

class IComponentArray {
  public:
    virtual ~IComponentArray() = default;
    virtual void objectDestroyed(ObjectID object) = 0;
    virtual void clearComponents() = 0;
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

    void clearComponents() override {
        return m_Map.clear();
    }

    bool hasComponent(ObjectID id) {
        return m_Map.contains(id);
    }

    T &getComponent(ObjectID id) {
        Logger::ASSERT(m_Map.contains(id), "Component Does Not Exist.");
        return m_Map.get(id);
    }

    std::vector<T> &getComponents() {
        return m_Map.getArray();
    }

    template <typename F>
        requires Comparator<F, T>
    void sortComponents(F comparator) {
        m_Map.sort(comparator);
    }

    void objectDestroyed(ObjectID id) override {
        removeComponent(id);
    }

  private:
    ContiguousMap<ObjectID, T> m_Map;
};

template <typename T>
concept TComponentArray = std::derived_from<ComponentArray<T>, IComponentArray>;

class ComponentManager {

  public:
    // how any of these symbols are resolved is beyond me.
    template <typename T>
        requires TComponentArray<T>
    void setComponent(T &component, ObjectID object) {
        getComponentArray<T>()->setComponent(component, object);
    }

    template <typename T>
        requires TComponentArray<T>
    bool removeComponent(ObjectID object) {
        return getComponentArray<T>()->removeComponent(object);
    }

    template <typename T>
        requires TComponentArray<T>
    bool hasComponent(ObjectID id) {
        return getComponentArray<T>()->hasComponent(id);
    }

    template <typename T>
        requires TComponentArray<T>
    bool existsComponents() {
        return m_Components.contains(typeid(T).name());
    }

    template <typename T>
        requires TComponentArray<T>
    T &getComponent(ObjectID object) {
        return getComponentArray<T>()->getComponent(object);
    }

    template <typename T>
        requires TComponentArray<T>
    std::vector<T> &getComponents() {
        return getComponentArray<T>()->getComponents();
    }

    template <typename T, typename F>
        requires Comparator<F, T>
    void sortComponents(F comparator) {
        return getComponentArray<T>()->sortComponents(comparator);
    }

    void objectDestroyed(ObjectID id) {
        for (const auto &pair : m_Components) {
            pair.second->objectDestroyed(id);
        }
    }

    void clearComponents() {
        for (const auto &pair : m_Components) {
            pair.second->clearComponents();
        }
    }

  private:
    template <typename T>
        requires TComponentArray<T>
    std::shared_ptr<ComponentArray<T>> getComponentArray() {
        std::string name = typeid(T).name();

        if (!m_Components.contains(name)) {
            m_Components[name] = std::make_shared<ComponentArray<T>>();
        }

        return std::static_pointer_cast<ComponentArray<T>>(m_Components[name]);
    }

    std::unordered_map<std::string, std::shared_ptr<IComponentArray>> m_Components;
};

} // namespace Physbuzz
