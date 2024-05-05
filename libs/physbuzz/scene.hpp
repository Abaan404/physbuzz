#pragma once

#include "component.hpp"
#include "defines.hpp"
#include "object.hpp"

namespace Physbuzz {

class Scene {
  public:
    ObjectID createObject();
    void deleteObject(ObjectID id);
    bool hasObject(ObjectID id);

    Object &getObject(ObjectID id);
    std::vector<Object> &getObjects();

    template <typename T>
    std::vector<T> &getComponents() {
        return m_ComponentManager.getComponents<T>();
    }

  private:
    ComponentManager m_ComponentManager;
    ObjectManager m_ObjectManager;
};

} // namespace Physbuzz
