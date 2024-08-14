#pragma once

#include "../ecs/system.hpp"
#include "../events/handler.hpp"
#include "../events/scene.hpp"

namespace Physbuzz {

class Scene : public EventSubject {
  public:
    ObjectID createObject(ObjectID id);
    bool eraseObject(ObjectID id);
    bool containsObject(ObjectID id) const;
    const std::set<ObjectID> &getObjects();

    template <typename... T>
    inline void setComponent(ObjectID id, T &...component) {
        m_ComponentManager.set<T...>(id, component...);
        m_SystemManager.objectUpdate(m_ComponentManager, id);

        (notifyCallbacks<OnComponentSetEvent<T>>(OnComponentSetEvent<T>{
             .scene = this,
             .object = id,
             .component = &getComponent<T>(id)}),
         ...);
    }

    template <typename... T>
    inline bool eraseComponent(ObjectID id) {
        if (containsComponent<T...>(id)) {
            (notifyCallbacks<OnComponentEraseEvent<T>>(OnComponentEraseEvent<T>{
                 .scene = this,
                 .object = id,
                 .component = &getComponent<T>(id)}),
             ...);
        }

        if (!m_ComponentManager.erase<T...>(id)) {
            return false;
        }

        m_SystemManager.objectUpdate(m_ComponentManager, id);
        return true;
    }

    template <typename T>
    inline T &getComponent(ObjectID id) {
        return m_ComponentManager.get<T>(id);
    }

    template <typename T>
    inline std::vector<T> &getComponents() {
        return m_ComponentManager.getArray<T>();
    }

    template <typename... T>
    inline bool containsComponent(ObjectID id) {
        return m_ComponentManager.contains<T...>(id);
    }

    template <typename T, typename... Args>
    inline std::shared_ptr<T> emplaceSystem(Args &&...args) {
        std::shared_ptr<T> system = m_SystemManager.emplace<T>(std::forward<Args>(args)...);

        notifyCallbacks<OnSystemEmplaceEvent<T>>({
            .scene = this,
            .system = system,
        });

        return system;
    }

    template <typename T>
    inline bool eraseSystem() {
        if (m_SystemManager.contains<T>()) {
            notifyCallbacks<OnSystemEraseEvent<T>>({
                .scene = this,
                .system = m_SystemManager.get<T>(),
            });
        }

        return m_SystemManager.erase<T>();
    }

    template <typename T>
    inline bool containsSystem() {
        return m_SystemManager.contains<T>();
    }

    template <typename T>
    inline std::shared_ptr<T> getSystem() {
        return m_SystemManager.get<T>();
    }

    template <typename... T>
    inline void tickSystem() {
        m_SystemManager.tick<T...>(*this);
    }

    void buildSystems();
    void destroySystems();

    void clear();

  private:
    // Note: Sparse sets for better locality?
    ComponentManager m_ComponentManager;
    SystemManager m_SystemManager;

    std::set<ObjectID> m_Objects;
};

} // namespace Physbuzz
