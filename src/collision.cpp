#include "collision.hpp"

Collision::Collision(Physbuzz::Scene *scene, const float restitution)
    : m_DetectorNarrow(scene), m_DetectorBroad(scene, &m_Objects), m_Resolver(scene, restitution) {}

Collision::~Collision() {}

void Collision::build() {
    m_DetectorBroad.build();
    m_DetectorNarrow.build();
}

void Collision::destroy() {
    m_DetectorBroad.destroy();
    m_DetectorNarrow.destroy();
}

void Collision::tick(Physbuzz::Scene &scene) {
    std::list<Physbuzz::Contact> contacts = m_DetectorBroad.find();
    m_DetectorNarrow.find(contacts);

    for (auto &contact : contacts) {
        m_Resolver.solve(contact);
    }
}
