#include "system.hpp"

namespace Physbuzz {

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

void SystemManager::clear() {
    for (auto &[_, system] : m_Systems) {
        if (!system->destroy()) {
            Logger::ERROR("[Scene/Systems] Failed to destroy a system.");
        }
    }

    m_Systems.clear();
}

} // namespace Physbuzz
