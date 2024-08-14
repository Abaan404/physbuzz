#include "scene.hpp"

#include "../events/scene.hpp"

namespace Physbuzz {

void Scene::clear() {
    notifyCallbacks<OnSceneClear>({
        .scene = this,
    });

    m_SystemManager.destroySystems();
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

void Scene::buildSystems() {
    m_SystemManager.buildSystems();
}

void Scene::destroySystems() {
    m_SystemManager.destroySystems();
}

} // namespace Physbuzz
