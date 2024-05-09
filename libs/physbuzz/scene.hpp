#pragma once

#include "component.hpp"
#include "defines.hpp"
#include "object.hpp"

namespace Physbuzz {

class Scene {
  public:
    ObjectID createObject();
    ObjectID createObject(ObjectID id);
    bool deleteObject(ObjectID id);
    bool hasObject(ObjectID id);

    Object &getObject(ObjectID id);
    std::vector<Object> &getObjects();

    void clear() {
        m_ComponentManager.clearComponents();
        m_ObjectManager.clearObjects();
    }

    template <typename T>
    bool existsComponents() {
        return m_ComponentManager.existsComponents<T>();
    }

    template <typename T>
    std::vector<T> &getComponents() {
        return m_ComponentManager.getComponents<T>();
    }

    template <typename T, typename F>
        requires Comparator<F, T>
    void sortComponents(F comparator) {
        return m_ComponentManager.sortComponents(comparator);
    }

    template <typename F>
        requires Comparator<F, Physbuzz::Object>
    void sortObjects(F comparator) {
        return m_ObjectManager.sortObjects(comparator);
    }

  private:
    ComponentManager m_ComponentManager;
    ObjectManager m_ObjectManager;
};

} // namespace Physbuzz
