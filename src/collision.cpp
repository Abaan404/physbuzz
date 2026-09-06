#include "collision.hpp"

Collision::Collision(Physbuzz::Scene *scene, const float restitution)
    : Physbuzz::System<Physbuzz::RigidBodyComponent, Physbuzz::RenderComponent>(scene),
      m_DetectorBroad(m_Scene, &m_Objects), m_DetectorNarrow(m_Scene), m_Resolver(m_Scene, restitution) {}

Collision::~Collision() {}

bool Collision::build() {
    bool success = true;

    success &= m_DetectorBroad.build();
    success &= m_DetectorNarrow.build();
    success &= m_Resolver.build();

    return success;
}

bool Collision::destroy() {
    bool success = true;

    success &= m_DetectorBroad.destroy();
    success &= m_DetectorNarrow.destroy();
    success &= m_Resolver.destroy();

    return success;
}

void Collision::tick() {
    m_Contacts = m_DetectorBroad.findBroad();
    m_DetectorNarrow.findNarrow(m_Contacts);

    for (auto &contact : m_Contacts) {
        m_Resolver.solve(contact);
    }
}

const std::vector<Physbuzz::Contact> &Collision::getContacts() const {
    return m_Contacts;
}
