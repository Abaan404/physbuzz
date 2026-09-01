#include "sweepandprune.hpp"

#include "../../ecs/scene.hpp"

namespace Physbuzz {

SweepAndPrune2D::SweepAndPrune2D(Scene *scene, std::unordered_set<ObjectID> *objects)
    : CollisionDetector(scene), m_Objects(objects) {}

bool SweepAndPrune2D::buildImpl() {
    return true;
}

bool SweepAndPrune2D::destroyImpl() {
    return true;
}

std::vector<Contact> SweepAndPrune2D::findBroadImpl() {
    std::vector<Contact> contacts;

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

bool SweepAndPrune2D::checkImpl(Contact &contact) {
    const auto [bounding1] = m_Scene->getComponent<BoundingComponent>(contact.object1);
    const auto [bounding2] = m_Scene->getComponent<BoundingComponent>(contact.object2);

    const AABB::Info &aabb1 = bounding1.aabb.getInfo();
    const AABB::Info &aabb2 = bounding2.aabb.getInfo();

    return aabb1.max.x > aabb2.min.x && aabb2.max.x > aabb1.min.x && aabb1.max.y > aabb2.min.y && aabb2.max.y > aabb1.min.y;
}

std::list<SweepEdge> SweepAndPrune2D::getEdges() {
    std::list<SweepEdge> edges;

    auto comparator = [](const SweepEdge &a, const SweepEdge &b) {
        return a.edge < b.edge;
    };

    for (const auto &object : *m_Objects) {
        const auto [bounding] = m_Scene->getComponent<BoundingComponent>(object);
        const AABB::Info &aabb = bounding.aabb.getInfo();

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

} // namespace Physbuzz
