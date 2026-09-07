#pragma once

#include "../misc/signature.hpp"
#include "component.hpp"
#include <concepts>
#include <unordered_set>

namespace Physbuzz {

class Scene;

class ISystem {
  public:
    ISystem(Scene *scene)
        : m_Scene(scene) {}

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
  public:
    System(Scene *scene)
        : ISystem(scene) {}

  private:
    inline bool containsSignature(ComponentManager &componentManager, ObjectID id) final {
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
    inline T *emplace(Scene *scene, Args &&...args) {
        if (contains<T>()) {
            Logger::WARNING("[Scene/Systems] System already exists, ignoring new.");
            return get<T>();
        }

        SignatureID id = Signature::ID<T>();

        if (m_Systems.size() <= id) {
            m_Systems.resize(id + 1);
        }

        std::unique_ptr<T> system = std::make_unique<T>(scene, std::forward<Args>(args)...);
        if (!system->build()) {
            Logger::ERROR("[Scene/Systems] Failed to build a system.");
            return nullptr;
        }

        m_Systems[id] = std::move(system);
        return get<T>();
    }

    template <SystemType... T>
    inline void erase() {
        if (!contains<T...>()) {
            return;
        }

        ([this] {
            SignatureID id = Signature::ID<T>();

            if (!m_Systems[id]->destroy()) {
                Logger::ERROR("[Scene/Systems] Failed to destroy a system.");
            }

            m_Systems[id].reset();
        }(),
         ...);
    }

    template <SystemType... T>
    inline bool contains() const {
        return ((m_Systems.size() > Signature::ID<T>() && m_Systems.at(Signature::ID<T>())) && ...);
    }

    template <SystemType T>
    inline T *get() const {
        if (!contains<T>()) {
            Logger::CRITICAL("[Scene] System not found.");
            return nullptr;
        }

        SignatureID id = Signature::ID<T>();
        return static_cast<T *>(m_Systems.at(id).get());
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
    std::vector<std::unique_ptr<ISystem>> m_Systems;
};

} // namespace Physbuzz
