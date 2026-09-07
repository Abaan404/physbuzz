#include "system.hpp"

namespace Physbuzz {

void SystemManager::objectUpdate(ComponentManager &componentManager, ObjectID id) {
    for (auto &system : m_Systems) {
        if (!system) {
            continue;
        }

        if (system->containsSignature(componentManager, id)) {
            system->m_Objects.insert(id);
        } else {
            system->m_Objects.erase(id);
        }
    }
}

void SystemManager::objectDestroyed(ObjectID id) {
    for (auto &system : m_Systems) {
        if (!system) {
            continue;
        }

        system->m_Objects.erase(id);
    }
}

void SystemManager::clear() {
    for (auto &system : m_Systems) {
        if (!system) {
            continue;
        }

        if (!system->destroy()) {
            Logger::ERROR("[Scene/Systems] Failed to destroy a system.");
        }
    }

    m_Systems.clear();
}

} // namespace Physbuzz
