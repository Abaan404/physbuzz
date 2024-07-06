#include "scene.hpp"
#include "physbuzz/events/scene.hpp"

namespace Physbuzz {

ObjectID Scene::createObject() {
    ObjectID id = m_ObjectManager.createObject(m_ComponentManager);
    notifyCallbacks<OnObjectCreateEvent>({
        .scene = this,
        .id = id,
    });

    return id;
}

ObjectID Scene::createObject(ObjectID id) {
    m_ObjectManager.createObject(m_ComponentManager, id);
    notifyCallbacks<OnObjectCreateEvent>({
        .scene = this,
        .id = id,
    });

    return id;
}

bool Scene::deleteObject(ObjectID id) {
    notifyCallbacks<OnObjectDeleteEvent>({
        .scene = this,
        .id = id,
    });

    return m_ObjectManager.deleteObject(m_ComponentManager, id);
}

bool Scene::hasObject(ObjectID id) const {
    return m_ObjectManager.hasObject(id);
}

Object &Scene::getObject(ObjectID id) {
    return m_ObjectManager.getObject(id);
}

std::vector<Object> &Scene::getObjects() {
    return m_ObjectManager.getObjects();
}

void Scene::clear() {
    m_ObjectManager.clearObjects();
    m_ComponentManager.clearComponents();
    m_EventMap.clear();
}

} // namespace Physbuzz
