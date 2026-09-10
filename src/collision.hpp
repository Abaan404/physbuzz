#pragma once

#include <physbuzz/render/defines.hpp>
#include <physbuzz/physics/components/rigidbody.hpp>
#include <physbuzz/physics/detectors/gjk.hpp>
#include <physbuzz/physics/detectors/sweepandprune.hpp>
#include <physbuzz/physics/resolvers/angular.hpp>

class Collision : public Physbuzz::System<Physbuzz::RigidBodyComponent, Physbuzz::TransformComponent> {
  public:
    Collision(Physbuzz::Scene *scene, const float restitution);
    ~Collision();

    bool build() override;
    bool destroy() override;

    void tick();

    const std::vector<Physbuzz::Contact> &getContacts() const;

  protected:
    Physbuzz::SweepAndPrune2D m_DetectorBroad;
    Physbuzz::Gjk3D m_DetectorNarrow;

    Physbuzz::AngularResolver2D m_Resolver;

    std::vector<Physbuzz::Contact> m_Contacts;
};
