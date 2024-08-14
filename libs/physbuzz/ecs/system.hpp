#pragma once

#include "../misc/signature.hpp"
#include "component.hpp"
#include <concepts>
#include <set>

namespace Physbuzz {

class Scene;

class ISystem {
  public:
    virtual void build();
    virtual void destroy();

  protected:
    std::set<ObjectID> m_Objects;

  private:
    virtual inline bool containsSignature(ComponentManager &componentManager, ObjectID id) = 0;
    friend class SystemManager;
};

template <typename... Signature>
class System : public ISystem {
  private:
    inline bool containsSignature(ComponentManager &componentManager, ObjectID id) override {
        return componentManager.contains<Signature...>(id);
    }
};

template <typename T>
concept SystemType = std::derived_from<T, ISystem>;

template <typename T>
concept SystemTickable =
    SystemType<T> &&
    requires(T a, Scene &scene, ObjectID id) {
        { a.tick(scene) } -> std::same_as<void>; // TODO multithread ticking system
    };

class SystemManager {
  public:
    void buildSystems();
    void destroySystems();

    template <SystemType T, typename... Args>
    inline std::shared_ptr<T> emplace(Args &&...system) {
        SignatureID id = Signature::ID<T>();

        if (!m_Systems.contains(id)) {
            m_Systems[id] = std::make_shared<T>(std::forward<Args>(system)...);
        }

        return get<T>();
    }

    template <SystemType... T>
    inline bool erase() {
        if (!contains<T...>()) {
            return false;
        }

        return (m_Systems.erase(Signature::ID<T>()) && ...);
    }

    template <SystemType... T>
    inline bool contains() {
        return (m_Systems.contains(Signature::ID<T>()) && ...);
    }

    template <SystemType T>
    inline std::shared_ptr<T> get() {
        SignatureID id = Signature::ID<T>();
        return std::static_pointer_cast<T>(m_Systems[id]);
    }

    template <SystemTickable... T>
    inline void tick(Scene &scene) {
        if (contains<T...>()) {
            (..., get<T>()->tick(scene));
        }
    }

    void objectUpdate(ComponentManager &componentManager, ObjectID id);
    void objectDestroyed(ObjectID id);

  private:
    std::unordered_map<SignatureID, std::shared_ptr<ISystem>> m_Systems;
};

} // namespace Physbuzz
