#pragma once

#include "../misc/signature.hpp"
#include "component.hpp"
#include <concepts>
#include <unordered_set>

namespace Physbuzz {

class Scene;

class ISystem {
  public:
    virtual ~ISystem() = default;
    virtual bool build() { return true; }
    virtual bool destroy() { return true; }

  protected:
    std::unordered_set<ObjectID> m_Objects;
    Scene *m_Scene = nullptr;

  private:
    virtual inline bool containsSignature(ComponentManager &componentManager, ObjectID id) = 0;
    friend class SystemManager;
};

template <typename... Signature>
class System : public ISystem {
  private:
    inline bool containsSignature(ComponentManager &componentManager, ObjectID id) override {
        if constexpr (sizeof...(Signature) == 0) {
            return false;
        } else {
            return componentManager.contains<Signature...>(id);
        }
    }

    friend class Scene;
};

template <typename T>
concept SystemType = std::derived_from<T, ISystem>;

template <typename T, typename... Args>
concept SystemTickable =
    SystemType<T> &&
    requires(T a, Args... args) {
        { a.tick(args...) };
    };

class SystemManager {
  public:
    template <SystemType T, typename... Args>
    inline std::shared_ptr<T> emplace(Scene *scene, Args &&...system) {
        SignatureID id = Signature::ID<T>();

        if (!m_Systems.contains(id)) {
            m_Systems[id] = std::make_shared<T>(std::forward<Args>(system)...);
            m_Systems[id]->m_Scene = scene;
            if (!m_Systems[id]->build()) {
                Logger::ERROR("[Scene/Systems] Failed to build a system.");
            }
        }

        return get<T>();
    }

    template <SystemType... T>
    inline bool erase() {
        if (!contains<T...>()) {
            return false;
        }

        (m_Systems[Signature::ID<T>()]->destroy(), ...);
        return (m_Systems.erase(Signature::ID<T>()) && ...);
    }

    template <SystemType... T>
    inline bool contains() {
        return (m_Systems.contains(Signature::ID<T>()) && ...);
    }

    template <SystemType T>
    inline std::shared_ptr<T> get() const {
        SignatureID id = Signature::ID<T>();
        PBZ_ASSERT(m_Systems.contains(id), "[Scene] System not found.");
        return std::static_pointer_cast<T>(m_Systems.at(id));
    }

    template <typename T, typename... Args>
        requires SystemTickable<T, Args...>
    inline void tick(Args &&...args) {
        if (contains<T>()) {
            get<T>()->tick(std::forward<Args>(args)...);
        }
    }

    void clear();
    void objectUpdate(ComponentManager &componentManager, ObjectID id);
    void objectDestroyed(ObjectID id);

  private:
    std::unordered_map<SignatureID, std::shared_ptr<ISystem>> m_Systems;
};

} // namespace Physbuzz
