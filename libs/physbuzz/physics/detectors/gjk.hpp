#pragma once

#include "../../render/defines.hpp"
#include "../collision.hpp"
#include <initializer_list>

namespace Physbuzz {

class Simplex {
  public:
    Simplex();
    Simplex(std::initializer_list<glm::vec3> list);

    const glm::vec3 &operator[](const std::size_t &idx) const;

    void pushFront(const glm::vec3 &point);

    std::size_t size() const;
    constexpr std::size_t maxSize() const {
        return m_Points.max_size();
    }

    auto begin() const;
    auto end() const;

  private:
    std::array<glm::vec3, 4> m_Points;
    std::size_t m_Size;
};

class Gjk3D : public CollisionDetector<Gjk3D> {
  public:
    Gjk3D(Scene *scene);

  protected:
    bool checkImpl(Contact &contact);

  private:
    bool buildImpl();
    bool destroyImpl();

    using CollisionDetector::findBroadImpl;
    using CollisionDetector::findNarrowImpl;

    void Epa(Simplex &simplex, Contact &contact, const RigidBodyComponent &body1, const RenderComponent &render1, const RigidBodyComponent &body2, const RenderComponent &render2);

    bool nextSimplex(Simplex &simplex, glm::vec3 &direction);

    bool line(Simplex &simplex, glm::vec3 &direction);
    bool triangle(Simplex &simplex, glm::vec3 &direction);
    bool tetrahedron(Simplex &simplex, glm::vec3 &direction);

    glm::vec3 supportPoint(const RigidBodyComponent &body, const RenderComponent &render, const glm::vec3 &direction);
    glm::vec3 minkowskiSupportPoint(const RigidBodyComponent &body1, const RenderComponent &render1, const RigidBodyComponent &body2, const RenderComponent &render2, const glm::vec3 &direction);

    std::pair<std::vector<glm::vec4>, std::size_t> getFaceNormals(const std::vector<glm::vec3> &polytope, const std::vector<Index> &faces);
    void AddIfUniqueEdge(std::vector<std::pair<Index, Index>> &edges, const std::vector<Index> &faces, std::size_t a, std::size_t b);

    bool isFacing(const glm::vec3 &vec, const glm::vec3 &direction);

    friend class CollisionDetector<Gjk3D>;
};

} // namespace Physbuzz
