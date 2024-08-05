#include "sweepandprune.hpp"

#include "physbuzz/events/object.hpp"
#include "physbuzz/events/scene.hpp"
#include "physbuzz/scene.hpp"
#include <algorithm>

namespace Physbuzz {

SweepAndPrune2D::SweepAndPrune2D(Scene &scene)
    : ICollisionDetector(scene) {}

void SweepAndPrune2D::build() {
    std::vector<Object> objects = m_Scene.getObjects();

    if (!objects.empty()) {
        for (Object &object : objects) {
            if (object.hasComponent<AABBComponent>()) {
                m_CollisionObjects.insert(object.getId());
            }
        }
    }

    m_ObjectAddEventId = m_Scene.addCallback<OnObjectCreateEvent>([&](const OnObjectCreateEvent &event) {
        if (&m_Scene != event.scene) {
            return;
        }

        Object &object = event.scene->getObject(event.id);

        m_ComponentEventIds[event.id][0] = object.addCallback<OnComponentSetEvent<AABBComponent>>([&](const OnComponentSetEvent<AABBComponent> &event) {
            m_CollisionObjects.insert(event.object->getId());
        });

        m_ComponentEventIds[event.id][1] = object.addCallback<OnComponentRemoveEvent<AABBComponent>>([&](const OnComponentRemoveEvent<AABBComponent> &event) {
            m_ComponentEventIds.erase(event.object->getId());
            const auto &it = std::find(m_CollisionObjects.begin(), m_CollisionObjects.end(), event.object->getId());
            if (it != m_CollisionObjects.end()) {
                m_CollisionObjects.erase(it);
            }
        });

        m_ComponentEventIds[event.id][2] = object.addCallback<OnComponentEraseEvent>([&](const OnComponentEraseEvent &event) {
            m_ComponentEventIds.erase(event.object->getId());
            const auto &it = std::find(m_CollisionObjects.begin(), m_CollisionObjects.end(), event.object->getId());
            if (it != m_CollisionObjects.end()) {
                m_CollisionObjects.erase(it);
            }
        });
    });

    m_ObjectRemoveEventId = m_Scene.addCallback<OnObjectDeleteEvent>([&](const OnObjectDeleteEvent &event) {
        if (&m_Scene != event.scene) {
            return;
        }

        m_ComponentEventIds.erase(event.id);
        const auto &it1 = std::find(m_CollisionObjects.begin(), m_CollisionObjects.end(), event.id);
        if (it1 != m_CollisionObjects.end()) {
            m_CollisionObjects.erase(it1);
        }
    });
}

void SweepAndPrune2D::destroy() {
    for (const auto &[objectId, eventIds] : m_ComponentEventIds) {
        Object &object = m_Scene.getObject(objectId);

        object.removeCallback<OnComponentSetEvent<AABBComponent>>(eventIds[0]);
        object.removeCallback<OnComponentRemoveEvent<AABBComponent>>(eventIds[1]);
        object.removeCallback<OnComponentEraseEvent>(eventIds[2]);
    }

    m_Scene.removeCallback<OnObjectCreateEvent>(m_ObjectAddEventId);
    m_Scene.removeCallback<OnObjectDeleteEvent>(m_ObjectRemoveEventId);
    m_ComponentEventIds.clear();
}

std::list<Contact> SweepAndPrune2D::find() {
    std::list<Contact> contacts;

    std::list<SweepEdge> edges = getEdges();
    std::list<SweepEdge> inSweep;

    for (const auto &edge1 : edges) {
        if (edge1.isLeft) {
            for (const auto &edge2 : inSweep) {
                if (edge1.id == edge2.id) {
                    continue;
                }

                Contact contact = {
                    .object1 = edge1.id,
                    .object2 = edge2.id,
                };

                if (check(contact)) {
                    contacts.push_back(std::move(contact));
                }
            }

            inSweep.emplace_back(edge1);
        } else {
            const auto &it = std::find_if(inSweep.begin(), inSweep.end(), [&edge1](const SweepEdge &edge) {
                return edge.id == edge1.id;
            });
            if (it != inSweep.end()) {
                inSweep.erase(it);
            }
        }
    }

    return contacts;
}

std::list<SweepEdge> SweepAndPrune2D::getEdges() {
    // figure out a better way to store edges instead of allocating new edges everytime
    std::list<SweepEdge> edges;

    auto comparator = [](const SweepEdge &a, const SweepEdge &b) {
        return a.edge < b.edge;
    };

    for (const ObjectID &id : m_CollisionObjects) {
        Object &object = m_Scene.getObject(id);
        const AABBComponent &aabb = object.getComponent<AABBComponent>();

        const SweepEdge edgeLeft = {
            .id = object.getId(),
            .edge = aabb.min.x,
            .isLeft = true,
        };

        const SweepEdge edgeRight = {
            .id = object.getId(),
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
    Object &object1 = m_Scene.getObject(contact.object1);
    Object &object2 = m_Scene.getObject(contact.object2);

    const AABBComponent &aabb1 = object1.getComponent<AABBComponent>();
    const AABBComponent &aabb2 = object2.getComponent<AABBComponent>();

    return aabb1.max.x > aabb2.min.x && aabb2.max.x > aabb1.min.x && aabb1.max.y > aabb2.min.y && aabb2.max.y > aabb1.min.y;
}

} // namespace Physbuzz
