#pragma once

#include "component.hpp"
#include "defines.hpp"
#include "object.hpp"

namespace Physbuzz {

class Scene {
  public:
    Object &createObject();
    void deleteObject(ObjectID id);
    Object &getObject(ObjectID id);
    std::vector<Object> &getObjects();

    template <typename T>
    std::vector<T> &getComponents() {
        return componentManager.getComponents<T>();
    }

  private:
    ComponentManager componentManager;
    ObjectManager objectManager;
};

} // namespace Physbuzz
