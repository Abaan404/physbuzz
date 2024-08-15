#include "sweepandprune.hpp"

#include <algorithm>

namespace Physbuzz {

SweepAndPrune2D::SweepAndPrune2D(Scene *scene, std::set<ObjectID> *objects)
    : ICollisionDetector(scene), m_Objects(objects) {}

std::list<Contact> SweepAndPrune2D::find() {
    std::list<Contact> contacts;

    std::list<SweepEdge> edges = getEdges();
    std::list<SweepEdge> inSweep;

    for (const auto &edge1 : edges) {
        if (edge1.isLeft) {
            for (const auto &edge2 : inSweep) {
                if (edge1.object == edge2.object) {
                    continue;
                }

                Contact contact = {
                    .object1 = edge1.object,
                    .object2 = edge2.object,
                };

                if (check(contact)) {
                    contacts.push_back(contact);
                }
            }

            inSweep.emplace_back(edge1);
        } else {
            const auto &it = std::find_if(inSweep.begin(), inSweep.end(), [&edge1](const SweepEdge &edge) {
                return edge.object == edge1.object;
            });
            if (it != inSweep.end()) {
                inSweep.erase(it);
            }
        }
    }

    return contacts;
}

std::list<SweepEdge> SweepAndPrune2D::getEdges() {
    std::list<SweepEdge> edges;

    auto comparator = [](const SweepEdge &a, const SweepEdge &b) {
        return a.edge < b.edge;
    };

    for (const auto &object : *m_Objects) {
        const AABBComponent &aabb = m_Scene->getComponent<AABBComponent>(object);

        const SweepEdge edgeLeft = {
            .object = object,
            .edge = aabb.min.x,
            .isLeft = true,
        };

        const SweepEdge edgeRight = {
            .object = object,
            .edge = aabb.max.x,
            .isLeft = false,
        };

        auto idxLeft = std::lower_bound(edges.begin(), edges.end(), edgeLeft, comparator);
        auto idxRight = std::lower_bound(edges.begin(), edges.end(), edgeRight, comparator);

        edges.insert(idxLeft, edgeLeft);
        edges.insert(idxRight, edgeRight);
    }

    return edges;
}

bool SweepAndPrune2D::check(Contact &contact) {
    const AABBComponent &aabb1 = m_Scene->getComponent<AABBComponent>(contact.object1);
    const AABBComponent &aabb2 = m_Scene->getComponent<AABBComponent>(contact.object2);

    return aabb1.max.x > aabb2.min.x && aabb2.max.x > aabb1.min.x && aabb1.max.y > aabb2.min.y && aabb2.max.y > aabb1.min.y;
}

} // namespace Physbuzz
