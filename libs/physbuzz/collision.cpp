#include "collision.hpp"

#include "events/collision.hpp"
#include <physbuzz/logging.hpp>
#include <physbuzz/renderer.hpp>

namespace Physbuzz {

ICollisionDetector::ICollisionDetector(Scene &scene)
    : m_Scene(scene) {}

std::list<Contact> ICollisionDetector::find() {
    std::list<Contact> contacts;

    for (const auto &object1 : m_Scene.getObjects()) {
        for (const auto &object2 : m_Scene.getObjects()) {
            if (object1.getId() == object2.getId()) {
                continue;
            }

            Contact contact = {
                .object1 = object1.getId(),
                .object2 = object2.getId(),
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
            m_Scene.notifyCallbacks<OnCollisionDetectEvent>({
                .scene = m_Scene,
                .contact = *it,
            });
            ++it;
        } else {
            it = contacts.erase(it);
        }
    }
}

void ICollisionDetector::build() { return; }
void ICollisionDetector::destroy() { return; }

ICollisionResolver::ICollisionResolver(Scene &scene)
    : m_Scene(scene) {}

void ICollisionResolver::solve(std::list<Contact> &contacts) {
    for (const auto &contact : contacts) {
        m_Scene.notifyCallbacks<OnCollisionResolveEvent>({
            .scene = m_Scene,
            .contact = contact,
        });

        solve(contact);
    }
}

void ICollisionResolver::build() { return; }
void ICollisionResolver::destroy() { return; }

Collision::Collision(std::shared_ptr<ICollisionDetector> narrow, std::shared_ptr<ICollisionDetector> broad, std::shared_ptr<ICollisionResolver> resolver)
    : narrowDetector(narrow),
      broadDetector(broad),
      resolver(resolver) {}

Collision::~Collision() {}

void Collision::tick(Scene &scene) {
    std::list<Contact> contacts = broadDetector->find();
    narrowDetector->find(contacts);

    for (auto &contact : contacts) {
        resolver->solve(contact);
    }
}

void Collision::build() {
    narrowDetector->build();
    broadDetector->build();
}

void Collision::destroy() {
    narrowDetector->destroy();
    broadDetector->destroy();
}

} // namespace Physbuzz
