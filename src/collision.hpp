#pragma once

#include <physbuzz/physics/components/rigidbody.hpp>
#include <physbuzz/physics/detectors/mpr.hpp>
#include <physbuzz/physics/detectors/sweepandprune.hpp>
#include <physbuzz/physics/resolvers/simple.hpp>
#include <physbuzz/render/defines.hpp>

class Collision : public Physbuzz::System<Physbuzz::RigidBodyComponent, Physbuzz::TransformComponent> {
  public:
    struct Info {
        float restitution;
    };

    Collision(Physbuzz::Scene *scene, const Info &info);
    ~Collision();

    bool build() override;
    bool destroy() override;

    void tick();

    const std::vector<Physbuzz::Contact> &getContacts() const;

  protected:
    Physbuzz::SweepAndPrune2D m_DetectorBroad;
    Physbuzz::Mpr3D m_DetectorNarrow;

    Physbuzz::SimpleResolver3D m_Resolver;

    std::vector<Physbuzz::Contact> m_Contacts;
};
