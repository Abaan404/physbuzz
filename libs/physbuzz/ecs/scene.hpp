#pragma once

#include "../ecs/system.hpp"
#include "../events/handler.hpp"
#include "../events/scene.hpp"

namespace Physbuzz {

class Scene : public EventSubject {
  public:
    ObjectID createObject();
    ObjectID createObject(ObjectID id);
    bool eraseObject(ObjectID id);
    bool containsObject(ObjectID id) const;
    const std::set<ObjectID> &getObjects();

    template <typename... T>
    inline void setComponent(ObjectID id, T &...component) {
        if (!m_Objects.contains(id)) {
            createObject(id);
        }

        m_ComponentManager.set<T...>(id, component...);
        m_SystemManager.objectUpdate(m_ComponentManager, id);

        (notifyCallbacks<OnComponentSetEvent<T>>(OnComponentSetEvent<T>{
             .scene = this,
             .object = id,
             .component = &std::get<0>(getComponent<T>(id)),
         }),
         ...);
    }

    template <typename... T>
    inline bool eraseComponent(ObjectID id) {
        if (containsComponent<T...>(id)) {
            (notifyCallbacks<OnComponentEraseEvent<T>>({
                 .scene = this,
                 .object = id,
                 .component = &std::get<0>(getComponent<T>(id)),
             }),
             ...);
        }

        if (!m_ComponentManager.erase<T...>(id)) {
            return false;
        }

        m_SystemManager.objectUpdate(m_ComponentManager, id);
        return true;
    }

    template <typename... T>
    inline std::tuple<T &...> getComponent(ObjectID id) {
        return m_ComponentManager.get<T...>(id);
    }

    template <typename... T>
    std::vector<std::tuple<ObjectID, T &...>> getComponents() {
        return m_ComponentManager.getArray<T...>();
    }

    template <typename T>
    const std::vector<T> &getComponentArray() {
        return m_ComponentManager.getComponentArray<T>();
    }

    template <typename... T>
    inline bool containsComponent(ObjectID id) {
        return m_ComponentManager.contains<T...>(id);
    }

    template <SystemType T, typename... Args>
    inline std::shared_ptr<T> createSystem(Args &&...args) {
        std::shared_ptr<T> system = m_SystemManager.emplace<T>(this, std::forward<Args>(args)...);

        for (ObjectID id : getObjects()) {
            if (system->containsSignature(m_ComponentManager, id)) {
                system->m_Objects.insert(id);
            }
        }

        if (system) {
            notifyCallbacks<OnSystemCreateEvent<T>>({
                .scene = this,
                .system = system,
            });
        }

        return system;
    }

    template <SystemType T>
    inline bool eraseSystem() {
        if (m_SystemManager.contains<T>()) {
            notifyCallbacks<OnSystemEraseEvent<T>>({
                .scene = this,
                .system = m_SystemManager.get<T>(),
            });
        }

        return m_SystemManager.erase<T>();
    }

    template <SystemType T>
    inline bool containsSystem() {
        return m_SystemManager.contains<T>();
    }

    template <SystemType T>
    inline std::shared_ptr<T> getSystem() const {
        return m_SystemManager.get<T>();
    }

    template <typename... T, typename... Args>
        requires(SystemTickable<T, Args...> && ...)
    void tickSystem(Args &&...args) {
        (..., m_SystemManager.tick<T>(std::forward<Args>(args)...));
    }

    void buildSystems();
    void destroySystems();

    void clear();

  private:
    // Note: Sparse sets for better locality?
    ComponentManager m_ComponentManager;
    SystemManager m_SystemManager;

    std::set<ObjectID> m_Objects;
    ObjectID m_ObjectCounter = 0;
};

} // namespace Physbuzz
