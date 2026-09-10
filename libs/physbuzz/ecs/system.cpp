#include "system.hpp"

namespace Physbuzz {

void SystemManager::objectUpdate(ComponentManager &componentManager, ObjectID id) {
    for (auto &system : m_Systems) {
        if (!system) {
            continue;
        }

        std::vector<ObjectID> &objects = system->m_Objects;

        auto position = std::lower_bound(objects.begin(), objects.end(), id);
        bool contains = (position != objects.end()) && (*position == id);
        bool containsSignature = system->containsSignature(componentManager, id);

        if (containsSignature && !contains) {
            objects.insert(position, id);
        } else if (!containsSignature && contains) {
            objects.erase(position);
        }
    }
}

void SystemManager::objectDestroyed(ObjectID id) {
    for (auto &system : m_Systems) {
        if (!system) {
            continue;
        }

        std::vector<ObjectID> &objects = system->m_Objects;

        auto position = std::lower_bound(objects.begin(), objects.end(), id);
        bool contains = (position != objects.end()) && (*position == id);

        if (contains) {
            objects.erase(position);
        }
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
