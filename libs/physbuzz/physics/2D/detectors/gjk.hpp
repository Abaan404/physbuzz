#pragma once

#include "../../../render/mesh.hpp"
#include "../../collision.hpp"
#include <initializer_list>

namespace Physbuzz {

class Simplex {
  public:
    Simplex();
    Simplex(std::initializer_list<glm::vec3> list);

    const glm::vec3 &operator[](const std::size_t &idx) const;

    void pushFront(const glm::vec3 &point);

    const std::size_t size() const;
    constexpr std::size_t maxSize() const {
        return m_Points.max_size();
    }

    auto begin() const;
    auto end() const;

  private:
    std::array<glm::vec3, 4> m_Points;
    std::size_t m_Size;
};

class Gjk2D : public ICollisionDetector {
  public:
    Gjk2D(Scene *scene);

    bool check(Contact &contact) override;

  protected:
    static void Epa(Simplex &simplex, Contact &contact, const Mesh &mesh1, const Mesh &mesh2, const TransformComponent &transfrom1, const TransformComponent &transfrom2);

    static bool nextSimplex(Simplex &simplex, glm::vec3 &direction);

    static bool line(Simplex &simplex, glm::vec3 &direction);
    static bool triangle(Simplex &simplex, glm::vec3 &direction);

    static glm::vec3 supportPoint(const Mesh &mesh, const TransformComponent &transfrom, const glm::vec3 &direction);
    static glm::vec3 minkowskiSupportPoint(const Mesh &mesh1, const Mesh &mesh2, const TransformComponent &transfrom1, const TransformComponent &transfrom2, const glm::vec3 &direction);
    static bool isFacing(const glm::vec3 &vec, const glm::vec3 &direction);
};

} // namespace Physbuzz
