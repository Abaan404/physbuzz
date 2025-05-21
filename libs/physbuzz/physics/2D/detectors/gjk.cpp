#include "gjk.hpp"

#include "../../../debug/logging.hpp"
#include <limits>
#include <vector>

namespace Physbuzz {

Simplex::Simplex() {
    m_Size = 0;
}

Simplex::Simplex(std::initializer_list<glm::vec3> list) {
    if (list.size() >= maxSize()) {
        Logger::CRITICAL("[Gjk2D] 4D collision is not supported.");
    }

    std::size_t i = 0;
    for (const auto &vec : list) {
        m_Points[i++] = vec;
    }
    m_Size = i;
}

const std::size_t Simplex::size() const {
    return m_Size;
}

void Simplex::pushFront(const glm::vec3 &point) {
    m_Points = {point, m_Points[0], m_Points[1], m_Points[2]}; // discard m_Points[3]
    m_Size = std::min(m_Size + 1, maxSize());
}

const glm::vec3 &Simplex::operator[](const std::size_t &idx) const {
    if (idx < 0 || idx > size()) {
        Logger::CRITICAL("[Gjk2D] Invalid Index {}.", idx);
    }

    return m_Points[idx];
}

auto Simplex::begin() const {
    return m_Points.begin();
}

auto Simplex::end() const {
    return m_Points.begin() + m_Size;
}

Gjk2D::Gjk2D(Scene *scene)
    : ICollisionDetector(scene) {}

bool Gjk2D::check(Contact &contact) {
    const Mesh &mesh1 = m_Scene->getComponent<Mesh>(contact.object1);
    const Mesh &mesh2 = m_Scene->getComponent<Mesh>(contact.object2);

    const TransformComponent &transfrom1 = m_Scene->getComponent<TransformComponent>(contact.object1);
    const TransformComponent &transfrom2 = m_Scene->getComponent<TransformComponent>(contact.object2);

    glm::vec3 support = minkowskiSupportPoint(mesh1, mesh2, transfrom1, transfrom2, glm::vec3(0.0f, 1.0f, 0.0f));
    glm::vec3 direction = -support;

    Simplex simplex;
    simplex.pushFront(support);

    while (true) {
        support = minkowskiSupportPoint(mesh1, mesh2, transfrom1, transfrom2, direction);

        if (glm::dot(support, direction) <= 0) {
            return false;
        }

        simplex.pushFront(support);
        if (nextSimplex(simplex, direction)) {
            Epa(simplex, contact, mesh1, mesh2, transfrom1, transfrom2);
            return true;
        }
    }
}

bool Gjk2D::nextSimplex(Simplex &simplex, glm::vec3 &direction) {
    switch (simplex.size()) {
    case 2:
        return line(simplex, direction);
    case 3:
        return triangle(simplex, direction);
    }

    UNREACHABLE("Invalid Simplex.");
}

bool Gjk2D::line(Simplex &simplex, glm::vec3 &direction) {
    const glm::vec3 &a = simplex[0];
    const glm::vec3 &b = simplex[1];

    const glm::vec3 ab = b - a;
    const glm::vec3 ao = -a;

    if (isFacing(ab, ao)) {
        direction = cross(cross(ab, ao), ab);
    } else {
        simplex = {a}; // go towards origin in next iteration
        direction = ao;
    }

    return false;
}

bool Gjk2D::triangle(Simplex &simplex, glm::vec3 &direction) {
    const glm::vec3 &a = simplex[0];
    const glm::vec3 &b = simplex[1];
    const glm::vec3 &c = simplex[2];

    const glm::vec3 ab = b - a;
    const glm::vec3 ac = c - a;
    const glm::vec3 ao = -a;

    glm::vec3 nABC = glm::cross(ab, ac);

    if (isFacing(direction = glm::cross(nABC, ac), ao)) {
        return line(simplex = {a, c}, direction);
    } else if (isFacing(direction = glm::cross(ab, nABC), ao)) {
        return line(simplex = {a, b}, direction);
    }

    return true;
}

void Gjk2D::Epa(Simplex &simplex, Contact &contact, const Mesh &mesh1, const Mesh &mesh2, const TransformComponent &transfrom1, const TransformComponent &transfrom2) {
    std::vector<glm::vec3> polytope(simplex.begin(), simplex.end());

    std::size_t minIndex = 0;
    contact.depth = std::numeric_limits<float>::max();

    while (contact.depth == std::numeric_limits<float>::max()) {
        for (std::size_t i = 0; i < polytope.size(); i++) {
            std::size_t j = (i + 1) % polytope.size();
            const glm::vec3 tangent = polytope[j] - polytope[i];

            glm::vec3 normal = glm::normalize(glm::cross(tangent, glm::vec3(0.0f, 0.0f, 1.0f)));
            float depth = glm::dot(normal, polytope[i]);

            if (depth < 0.0f) {
                depth *= -1.0f;
                normal *= -1.0f;
            }

            if (depth < contact.depth) {
                minIndex = j;
                contact.depth = depth;
                contact.normal = normal;
            }
        }

        const glm::vec3 support1 = supportPoint(mesh1, transfrom1, contact.normal);
        const glm::vec3 support2 = supportPoint(mesh2, transfrom2, -contact.normal);
        const glm::vec3 support = support1 - support2;

        const float curDepth = glm::dot(contact.normal, support);

        if (glm::abs(curDepth - contact.depth) > 1e-4f) {
            contact.depth = std::numeric_limits<float>::max();
            polytope.insert(polytope.begin() + minIndex, support);
        }

        contact.point1 = support1;
        contact.point2 = support2;
    }

    contact.depth += 1e-4f;
}

// this might belong to a component than here, if I care about MPR (https://en.wikipedia.org/wiki/Minkowski_Portal_Refinement)
glm::vec3 Gjk2D::supportPoint(const Mesh &mesh, const TransformComponent &transfrom, const glm::vec3 &direction) {
    glm::vec3 point;
    float proj = std::numeric_limits<float>::lowest();

    for (const auto &vertex : mesh.vertices) {
        glm::vec3 position = transfrom.toWorld(vertex.position);

        const float newProj = glm::dot(position, direction);
        if (newProj > proj) {
            point = position;
            proj = newProj;
        }
    }

    return point;
}

glm::vec3 Gjk2D::minkowskiSupportPoint(const Mesh &mesh1, const Mesh &mesh2, const TransformComponent &transfrom1, const TransformComponent &transfrom2, const glm::vec3 &direction) {
    return supportPoint(mesh1, transfrom1, direction) - supportPoint(mesh2, transfrom2, -direction);
}

bool Gjk2D::isFacing(const glm::vec3 &vec, const glm::vec3 &direction) {
    return glm::dot(vec, direction) > 0.0f;
}

} // namespace Physbuzz
