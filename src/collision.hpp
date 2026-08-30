#pragma once

#include <physbuzz/physics/2D/detectors/gjk.hpp>
#include <physbuzz/physics/2D/detectors/sweepandprune.hpp>
#include <physbuzz/physics/2D/resolvers/angular.hpp>

class Collision : public Physbuzz::System<Physbuzz::RigidBodyComponent, Physbuzz::BoundingComponent, Physbuzz::RenderComponent> {
  public:
    Collision(Physbuzz::Scene *scene, const float restitution);
    ~Collision();

    bool build() override;
    bool destroy() override;

    void tick();

    const std::vector<Physbuzz::Contact> &getContacts() const;

  protected:
    Physbuzz::SweepAndPrune2D m_DetectorBroad;
    Physbuzz::Gjk2D m_DetectorNarrow;

    Physbuzz::AngularResolver2D m_Resolver;

    std::vector<Physbuzz::Contact> m_Contacts;
};
