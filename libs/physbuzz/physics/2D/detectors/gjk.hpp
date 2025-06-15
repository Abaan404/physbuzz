#pragma once

#include "../../collision.hpp"
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

class Gjk2D : public ICollisionDetector {
  public:
    Gjk2D(Scene *scene);

    bool check(Contact &contact) override;

  protected:
    void Epa(Simplex &simplex, Contact &contact, const RenderComponent &render1, const RenderComponent &render2);

    bool nextSimplex(Simplex &simplex, glm::vec3 &direction);

    bool line(Simplex &simplex, glm::vec3 &direction);
    bool triangle(Simplex &simplex, glm::vec3 &direction);

    glm::vec3 supportPoint(const RenderComponent &render, const glm::vec3 &direction);
    glm::vec3 minkowskiSupportPoint(const RenderComponent &render1, const RenderComponent &render2, const glm::vec3 &direction);
    bool isFacing(const glm::vec3 &vec, const glm::vec3 &direction);
};

} // namespace Physbuzz
