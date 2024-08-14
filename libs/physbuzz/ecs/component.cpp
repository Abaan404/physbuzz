#include "component.hpp"

namespace Physbuzz {

void ComponentManager::objectDestroyed(ObjectID id) {
    for (const auto &[_, components] : m_Components) {
        components->objectDestroyed(id);
    }
}

void ComponentManager::clear() {
    for (const auto &[_, components] : m_Components) {
        components->clear();
    }
}

} // namespace Physbuzz
