#include "collision.hpp"

#include "../ecs/scene.hpp"
#include "../events/collision.hpp"
#include "../render/model.hpp"
#include "dynamics.hpp"

namespace Physbuzz {

AABBComponent::AABBComponent(const RenderComponent &render, const RigidBodyComponent &body) {
    for (const auto &[mesh, _] : render.model->getMeshs()) {

        for (const auto &vertex : body.geometry.vertices) {
            min = glm::min(min, render.transform.toWorld(vertex.position));
            max = glm::max(max, render.transform.toWorld(vertex.position));
        }
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

bool ICollisionDetector::build() { return true; }
bool ICollisionDetector::destroy() { return true; }

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

bool ICollisionResolver::build() { return true; }
bool ICollisionResolver::destroy() { return true; }

} // namespace Physbuzz
