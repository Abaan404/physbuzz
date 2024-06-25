#include "sweepandprune.hpp"

namespace Physbuzz {

// TODO use the correct sweepandprune algorithm
std::list<Contact> SweepAndPrune2D::find(Scene &scene) {
    sortObjects(scene);

    std::list<Contact> contacts;
    for (const auto &[id1, aabb1] : m_SortedObjects) {
        for (const auto &[id2, aabb2] : m_SortedObjects) {
            if (id1 == id2) {
                continue;
            }

            if (aabb1.min.x > aabb2.max.x) {
                continue;
            }

            Contact contact = {
                .object1 = id1,
                .object2 = id2,
            };

            contacts.emplace_back(std::move(contact));
        }
    }

    return contacts;
}

void SweepAndPrune2D::reset() {
    m_SortedObjects.clear();
}

void SweepAndPrune2D::sortObjects(Scene &scene) {
    // TODO create eventhandlers to avoid recreating sorted array on adding new objects
    {
        m_SortedObjects.clear();

        for (auto &object : scene.getObjects()) {
            if (!object.hasComponent<BoundingComponent>()) {
                continue;
            }

            const BoundingComponent &aabb = object.getComponent<BoundingComponent>();
            m_SortedObjects.emplace_back(object.getId(), aabb.getBox());
        }
    }

    // TODO perf test
    std::sort(m_SortedObjects.begin(), m_SortedObjects.end(), [](const std::pair<ObjectID, AABBComponent> &obj1, const std::pair<ObjectID, AABBComponent> &obj2) {
        const AABBComponent &aabb1 = obj1.second;
        const AABBComponent &aabb2 = obj2.second;

        return aabb1.min.x < aabb2.min.x;
    });
}

bool SweepAndPrune2D::check(Scene &scene, Contact &contact) {
    Logger::ERROR("SweepAndPrune2D::check NotImplementedError");
    return false;
}

void SweepAndPrune2D::find(Scene &scene, std::list<Contact> &contacts) {
    Logger::ERROR("SweepAndPrune2D::find NotImplementedError");
}

} // namespace Physbuzz
