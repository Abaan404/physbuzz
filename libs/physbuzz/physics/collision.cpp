#include "collision.hpp"

#include "../ecs/scene.hpp"

namespace Physbuzz {

ICollisionResolver::ICollisionResolver(Scene *scene)
    : m_Scene(scene) {}

void ICollisionResolver::solve(std::list<Contact> &contacts) {
    for (const auto &contact : contacts) {
        solve(contact);
    }
}

bool ICollisionResolver::build() { return true; }
bool ICollisionResolver::destroy() { return true; }

} // namespace Physbuzz
