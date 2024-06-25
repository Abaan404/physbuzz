#include "collision.hpp"

#include <physbuzz/logging.hpp>
#include <physbuzz/renderer.hpp>

namespace Physbuzz {

Collision::Collision(std::shared_ptr<ICollisionDetector> narrow, std::shared_ptr<ICollisionDetector> broad, std::shared_ptr<ICollisionResolver> resolver)
    : narrowDetector(narrow),
      broadDetector(broad),
      resolver(resolver) {}

Collision::~Collision() {}

std::list<Contact> ICollisionDetector::find(Scene &scene) {
    std::list<Contact> contacts;

    for (const auto &object1 : scene.getObjects()) {
        for (const auto &object2 : scene.getObjects()) {
            if (object1.getId() == object2.getId()) {
                continue;
            }

            Contact contact = {
                .object1 = object1.getId(),
                .object2 = object2.getId(),
            };

            if (check(scene, contact)) {
                contacts.emplace_back(std::move(contact));
            }
        }
    }

    return contacts;
}

void ICollisionDetector::find(Scene &scene, std::list<Contact> &contacts) {
    auto iterator = contacts.begin();

    for (std::size_t i = 0; i < contacts.size(); i++) {
        Contact &contact = *iterator;
        iterator = std::next(iterator);

        // break the link chain behind the iterator
        if (check(scene, contact)) {
            contacts.erase(std::prev(iterator));
        }
    }
}

void ICollisionDetector::reset() {
    return;
}

void ICollisionResolver::solve(Scene &scene, std::list<Contact> &contacts) {
    for (const auto &contact : contacts) {
        solve(scene, contact);
    }
}

void ICollisionResolver::reset() {
    return;
}

void Collision::tick(Scene &scene) {
    std::list<Contact> contacts = broadDetector->find(scene);
    narrowDetector->find(scene, contacts);

    for (auto &contact : contacts) {
        resolver->solve(scene, contact);
    }
}

} // namespace Physbuzz
