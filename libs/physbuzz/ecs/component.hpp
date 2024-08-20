#pragma once

#include "../containers/contigiousmap.hpp"
#include "../misc/signature.hpp"
#include <memory>

namespace Physbuzz {

using ObjectID = std::uint32_t;

class IComponentArray {
  public:
    virtual ~IComponentArray() = default;
    virtual void objectDestroyed(ObjectID id) = 0;
    virtual void clear() = 0;
};

template <typename T>
class ComponentArray : public IComponentArray {
  public:
    inline void set(ObjectID id, T &component) {
        m_Map.insert(id, component);
    }

    inline bool erase(ObjectID id) {
        return m_Map.erase(id);
    }

    inline bool contains(ObjectID id) {
        return m_Map.contains(id);
    }

    inline T &get(ObjectID id) {
        return m_Map.get(id);
    }

    inline const std::vector<T> &getArray() {
        return m_Map.getArray();
    }

    void objectDestroyed(ObjectID id) override {
        erase(id);
    }

    void clear() override {
        m_Map.clear();
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
    inline void set(ObjectID id, T &...component) {
        (getComponents<T>()->set(id, component), ...);
    }

    template <ComponentArrayType... T>
    inline bool erase(ObjectID id) {
        // erase only all (&&) matches
        if (sizeof...(T) > 1 && !contains<T...>(id)) {
            return false;
        }

        return (getComponents<T>()->erase(id) && ...);
    }

    template <ComponentArrayType... T>
    inline bool contains(ObjectID id) {
        return (getComponents<T>()->contains(id) && ...);
    }

    template <ComponentArrayType T>
    inline T &get(ObjectID id) {
        return getComponents<T>()->get(id);
    }

    template <ComponentArrayType T>
    inline const std::vector<T> &getArray() {
        return getComponents<T>()->getArray();
    }

    void objectDestroyed(ObjectID id);
    void clear();

  private:
    template <ComponentArrayType T>
    std::shared_ptr<ComponentArray<T>> getComponents() {
        SignatureID id = Signature::ID<T>();

        if (!m_Components.contains(id)) {
            m_Components[id] = std::make_shared<ComponentArray<T>>();
        }

        return std::static_pointer_cast<ComponentArray<T>>(m_Components[id]);
    }

    std::unordered_map<SignatureID, std::shared_ptr<IComponentArray>> m_Components;
};

} // namespace Physbuzz
