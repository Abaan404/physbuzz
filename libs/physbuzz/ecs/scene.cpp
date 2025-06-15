#include "scene.hpp"

#include "../events/scene.hpp"

namespace Physbuzz {

void Scene::clear() {
    notifyCallbacks<OnSceneClear>({
        .scene = this,
    });

    m_SystemManager.clear();
    m_ComponentManager.clear();
    m_Objects.clear();
    clearCallbacks();
}

ObjectID Scene::createObject(ObjectID id) {
    if (m_Objects.contains(id)) {
        eraseObject(id);
    }

    m_Objects.insert(id);
    notifyCallbacks<OnObjectCreateEvent>({
        .scene = this,
        .object = id,
    });

    return id;
}

ObjectID Scene::createObject() {
    // keep counting until an unclaimed object is found
    while (containsObject(m_ObjectCounter)) {
        m_ObjectCounter++;
    }

    return createObject(m_ObjectCounter++);
}

bool Scene::eraseObject(ObjectID id) {
    if (!containsObject(id)) {
        return false;
    }

    notifyCallbacks<OnObjectEraseEvent>({
        .scene = this,
        .object = id,
    });

    m_ComponentManager.objectDestroyed(id);
    m_SystemManager.objectDestroyed(id);
    m_Objects.erase(id);

    return true;
}

bool Scene::containsObject(ObjectID id) const {
    return m_Objects.contains(id);
}

const std::set<ObjectID> &Scene::getObjects() {
    return m_Objects;
}

} // namespace Physbuzz
