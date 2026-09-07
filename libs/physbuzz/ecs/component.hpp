#pragma once

#include "../containers/contigiousmap.hpp"
#include "../misc/signature.hpp"
#include "defines.hpp"
#include <memory>
#include <tuple>

namespace Physbuzz {

class IComponentArray {
  public:
    virtual ~IComponentArray() = default;
    virtual void objectDestroyed(ObjectID id) = 0;
    virtual void clear() = 0;
};

template <typename T>
class ComponentArray : public IComponentArray {
  public:
    void set(ObjectID id, T &component) {
        m_Map.insert(id, component);
    }

    bool erase(ObjectID id) {
        return m_Map.erase(id);
    }

    bool contains(ObjectID id) const {
        return m_Map.contains(id);
    }

    T &get(ObjectID id) {
        return m_Map.get(id);
    }

    const T &get(ObjectID id) const {
        PBZ_ASSERT(m_Map.contains(id), "[Scene] Component not found.");
        return m_Map.get(id);
    }

    const std::vector<T> &getArray() const {
        return m_Map.getArray();
    }

    const std::unordered_set<ObjectID> &getKeys() const {
        return m_Map.getKeys();
    }

    void objectDestroyed(ObjectID id) override {
        erase(id);
    }

    void clear() override {
        m_Map.clear();
    }

    static const ComponentArray<T> &getEmpty() {
        static ComponentArray<T> Empty;
        return Empty;
    }

  private:
    ContiguousMap<ObjectID, T> m_Map;
};

template <typename... T>
concept ComponentArrayType =
    (sizeof...(T) > 0) &&
    (std::derived_from<ComponentArray<T>, IComponentArray> && ...);

class ComponentManager {
  public:
    template <ComponentArrayType... T>
    void set(ObjectID id, T &...component) {
        (getComponents<T>()->set(id, component), ...);
    }

    template <ComponentArrayType... T>
    bool erase(ObjectID id) {
        // erase only all (&&) matches
        if (sizeof...(T) > 1 && !contains<T...>(id)) {
            return false;
        }

        return (getComponents<T>()->erase(id) && ...);
    }

    template <ComponentArrayType... T>
    bool contains(ObjectID id) const {
        return (getComponents<T>()->contains(id) && ...);
    }

    template <ComponentArrayType... T>
    std::tuple<T &...> get(ObjectID id) {
        return std::tie(getComponents<T>()->get(id)...);
    }

    template <ComponentArrayType... T>
    std::tuple<const T &...> get(ObjectID id) const {
        return std::tie(getComponents<T>()->get(id)...);
    }

    template <ComponentArrayType... T>
    std::vector<std::tuple<ObjectID, T &...>> getArray() {
        std::unordered_set<ObjectID> ids;
        (ids.insert(getComponents<T>()->getKeys().begin(), getComponents<T>()->getKeys().end()), ...);

        std::vector<std::tuple<ObjectID, T &...>> result;
        for (const auto &id : ids) {
            if (contains<T...>(id)) {
                result.emplace_back(std::tuple_cat(std::make_tuple(id), get<T...>(id)));
            }
        }

        return result;
    }

    template <ComponentArrayType... T>
    std::vector<std::tuple<ObjectID, const T &...>> getArray() const {
        std::unordered_set<ObjectID> ids;
        (ids.insert(getComponents<T>()->getKeys().begin(), getComponents<T>()->getKeys().end()), ...);

        std::vector<std::tuple<ObjectID, const T &...>> result;
        for (const auto &id : ids) {
            if (contains<T...>(id)) {
                result.emplace_back(std::tuple_cat(std::make_tuple(id), get<T...>(id)));
            }
        }

        return result;
    }

    template <ComponentArrayType... T>
    std::unordered_set<ObjectID> getObjects() const {
        std::unordered_set<ObjectID> ids;
        (ids.insert(getComponents<T>()->getKeys().begin(), getComponents<T>()->getKeys().end()), ...);

        return ids;
    }

    template <ComponentArrayType T>
    const std::vector<T> &getComponentArray() const {
        return getComponents<T>()->getArray();
    }

    void objectDestroyed(ObjectID id);
    void clear();

  private:
    template <ComponentArrayType T>
    ComponentArray<T> *getComponents() {
        SignatureID id = Signature::ID<T>();

        if (m_Components.size() <= id) {
            m_Components.resize(id + 1);
        }

        if (!m_Components[id]) {
            m_Components[id] = std::make_unique<ComponentArray<T>>();
        }

        return static_cast<ComponentArray<T> *>(m_Components[id].get());
    }

    template <ComponentArrayType T>
    const ComponentArray<T> *getComponents() const {
        SignatureID id = Signature::ID<T>();

        if (m_Components.size() <= id || !m_Components[id]) {
            return &ComponentArray<T>::getEmpty();
        }

        return static_cast<const ComponentArray<T> *>(m_Components[id].get());
    }

    std::vector<std::unique_ptr<IComponentArray>> m_Components;
};

} // namespace Physbuzz
