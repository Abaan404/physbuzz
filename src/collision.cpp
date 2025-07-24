#include "collision.hpp"

Collision::Collision(Physbuzz::Scene *scene, const float restitution)
    : m_DetectorBroad(scene, &m_Objects), m_DetectorNarrow(scene), m_Resolver(scene, restitution) {}

Collision::~Collision() {}

bool Collision::build() {
    bool success = true;

    success &= m_DetectorBroad.build();
    success &= m_DetectorNarrow.build();

    return success;
}

bool Collision::destroy() {
    bool success = true;

    success &= m_DetectorBroad.destroy();
    success &= m_DetectorNarrow.destroy();

    return success;
}

void Collision::tick() {
    std::list<Physbuzz::Contact> contacts = m_DetectorBroad.find();
    m_DetectorNarrow.find(contacts);

    for (auto &contact : contacts) {
        m_Resolver.solve(contact);
    }
}
