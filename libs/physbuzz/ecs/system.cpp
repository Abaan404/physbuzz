#include "system.hpp"

namespace Physbuzz {

void ISystem::build() { return; }
void ISystem::destroy() { return; }

void SystemManager::objectUpdate(ComponentManager &componentManager, ObjectID id) {
    for (auto &[_, system] : m_Systems) {
        if (system->containsSignature(componentManager, id)) {
            system->m_Objects.insert(id);
        } else {
            system->m_Objects.erase(id);
        }
    }
}

void SystemManager::objectDestroyed(ObjectID id) {
    for (auto &[_, system] : m_Systems) {
        system->m_Objects.erase(id);
    }
}

void SystemManager::buildSystems() {
    for (auto &[_, system] : m_Systems) {
        system->build();
    }
}

void SystemManager::destroySystems() {
    for (auto &[_, system] : m_Systems) {
        system->destroy();
    }

    m_Systems.clear();
}

} // namespace Physbuzz
