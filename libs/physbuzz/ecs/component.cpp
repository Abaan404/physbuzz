#include "component.hpp"

namespace Physbuzz {

void ComponentManager::objectDestroyed(ObjectID id) {
    for (const auto &components : m_Components) {
        if (!components) {
            continue;
        }

        components->objectDestroyed(id);
    }
}

void ComponentManager::clear() {
    for (const auto &components : m_Components) {
        if (!components) {
            continue;
        }

        components->clear();
    }
}

} // namespace Physbuzz
