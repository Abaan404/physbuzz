#include "collision.hpp"

#include "../events/collision.hpp"

namespace Physbuzz {

AABBComponent::AABBComponent(const Mesh &mesh, const TransformComponent &transform) {
    for (const auto &vertex : mesh.vertices) {
        min = glm::min(min, transform.toWorld(vertex.position));
        max = glm::max(max, transform.toWorld(vertex.position));
    }
}

AABBComponent::AABBComponent(const std::vector<Mesh> &meshes, const TransformComponent &transform) {
    for (const auto &mesh : meshes) {
        AABBComponent aabb = AABBComponent(mesh, transform);

        min = glm::min(min, aabb.min);
        max = glm::max(max, aabb.max);
    }
}

ICollisionDetector::ICollisionDetector(Scene *scene)
    : m_Scene(scene) {}

std::list<Contact> ICollisionDetector::find() {
    std::list<Contact> contacts;

    for (const auto &object1 : m_Scene->getObjects()) {
        for (const auto &object2 : m_Scene->getObjects()) {
            if (object1 == object2) {
                continue;
            }

            Contact contact = {
                .object1 = object1,
                .object2 = object2,
            };

            if (check(contact)) {
                contacts.emplace_back(std::move(contact));
            }
        }
    }

    return contacts;
}

void ICollisionDetector::find(std::list<Contact> &contacts) {
    for (auto it = contacts.begin(); it != contacts.end();) {
        if (check(*it)) {
            notifyCallbacks<OnCollisionDetectEvent>({
                .scene = m_Scene,
                .contact = &*it, // what?
            });
            ++it;
        } else {
            it = contacts.erase(it);
        }
    }
}

void ICollisionDetector::build() { return; }
void ICollisionDetector::destroy() { return; }

ICollisionResolver::ICollisionResolver(Scene *scene)
    : m_Scene(scene) {}

void ICollisionResolver::solve(std::list<Contact> &contacts) {
    for (const auto &contact : contacts) {
        m_Scene->notifyCallbacks<OnCollisionResolveEvent>({
            .scene = m_Scene,
            .contact = &contact,
        });

        solve(contact);
    }
}

void ICollisionResolver::build() { return; }
void ICollisionResolver::destroy() { return; }

} // namespace Physbuzz
