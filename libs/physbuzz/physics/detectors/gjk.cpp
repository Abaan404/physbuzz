#include "gjk.hpp"

#include "../../debug/macros.hpp"
#include "../../ecs/scene.hpp"
#include "../../render/defines.hpp"
#include "../components/rigidbody.hpp"
#include <limits>
#include <vector>

// Completely adapted from
// - https://winter.dev/articles/epa-algorithm/
// - https://winter.dev/articles/gjk-algorithm/
// - https://www.youtube.com/watch?v=Qupqu1xe7Io (referenced)

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

std::size_t Simplex::size() const {
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

Gjk3D::Gjk3D(Scene *scene)
    : CollisionDetector(scene) {}

bool Gjk3D::buildImpl() {
    return true;
}

bool Gjk3D::destroyImpl() {
    return true;
}

bool Gjk3D::checkImpl(Contact &contact) {
    const auto [body1, transform1] = m_Scene->getComponent<RigidBodyComponent, TransformComponent>(contact.object1);
    const auto [body2, transform2] = m_Scene->getComponent<RigidBodyComponent, TransformComponent>(contact.object2);

    glm::vec3 support = minkowskiSupportPoint(body1, transform1.transform, body2, transform2.transform, glm::vec3(0.0f, 1.0f, 0.0f));
    glm::vec3 direction = -support;

    Simplex simplex;
    simplex.pushFront(support);

    while (true) {
        support = minkowskiSupportPoint(body1, transform1.transform, body2, transform2.transform, direction);

        if (glm::dot(support, direction) <= 0) {
            return false;
        }

        simplex.pushFront(support);
        if (nextSimplex(simplex, direction)) {
            Epa(simplex, contact, body1, transform1.transform, body2, transform2.transform);
            return true;
        }
    }
}

// https://www.youtube.com/watch?v=Qupqu1xe7Io
bool Gjk3D::nextSimplex(Simplex &simplex, glm::vec3 &direction) {
    switch (simplex.size()) {
    case 2:
        return line(simplex, direction);
    case 3:
        return triangle(simplex, direction);
    case 4:
        return tetrahedron(simplex, direction);
    }

    PBZ_UNREACHABLE("Invalid Simplex.");
}

bool Gjk3D::line(Simplex &simplex, glm::vec3 &direction) {
    const glm::vec3 &a = simplex[0];
    const glm::vec3 &b = simplex[1];

    const glm::vec3 ab = b - a;
    const glm::vec3 ao = -a;

    if (isFacing(ab, ao)) {
        // R_ab
        direction = glm::cross(glm::cross(ab, ao), ab);

        if (direction == glm::vec3(0.0f)) {
            // https://math.stackexchange.com/a/4112622
            direction = glm::vec3(
                copysign(ab.z, ab.x),
                copysign(ab.z, ab.y),
                copysign(abs(ab.x) + abs(ab.y), ab.z));
        }
    } else {
        // R_a
        simplex = {a}; // go towards origin in next iteration
        direction = ao;
    }

    return false;
}

bool Gjk3D::triangle(Simplex &simplex, glm::vec3 &direction) {
    const glm::vec3 &a = simplex[0];
    const glm::vec3 &b = simplex[1];
    const glm::vec3 &c = simplex[2];

    const glm::vec3 ab = b - a;
    const glm::vec3 ac = c - a;
    const glm::vec3 ao = -a;

    glm::vec3 abc = glm::cross(ab, ac);

    if (isFacing(glm::cross(abc, ac), ao)) {
        if (isFacing(ac, ao)) {
            // R_ac
            // plane normal is not the origin's direction, recompute
            simplex = {a, c};
            direction = glm::cross(glm::cross(ac, ao), ac);
        } else {
            // could be R_a or R_ab
            simplex = {a, b};
            return line(simplex, direction);
        }
    } else if (isFacing(glm::cross(ab, abc), ao)) {
        // could be R_a or R_ab
        simplex = {a, b};
        return line(simplex, direction);
    } else {
        if (isFacing(abc, ao)) {
            // R_abc (outwards)
            direction = abc;
        } else {
            // R_acb (inwards)
            simplex = {a, c, b};
            direction = -abc;
        }
    }

    return false;
}

bool Gjk3D::tetrahedron(Simplex &simplex, glm::vec3 &direction) {
    const glm::vec3 &a = simplex[0];
    const glm::vec3 &b = simplex[1];
    const glm::vec3 &c = simplex[2];
    const glm::vec3 &d = simplex[3];

    glm::vec3 ab = b - a;
    glm::vec3 ac = c - a;
    glm::vec3 ad = d - a;
    glm::vec3 ao = -a;

    glm::vec3 abc = glm::cross(ab, ac);
    glm::vec3 acd = glm::cross(ac, ad);
    glm::vec3 adb = glm::cross(ad, ab);

    if (isFacing(abc, ao)) {
        // R_abc
        simplex = {a, b, c};
        return triangle(simplex, direction);
    } else if (isFacing(acd, ao)) {
        // R_acd
        simplex = {a, c, d};
        return triangle(simplex, direction);
    } else if (isFacing(adb, ao)) {
        // R_adb
        simplex = {a, d, b};
        return triangle(simplex, direction);
    }

    return true;
}

// FIXME this can possibly cause infinite loops or segfault in rare cases
// i.e. spheres imposed on each other, normals being empty, etc. investigate further later
void Gjk3D::Epa(Simplex &simplex, Contact &contact, const RigidBodyComponent &body1, const Transform &transform1, const RigidBodyComponent &body2, const Transform &transform2) {
    std::vector<glm::vec3> polytope(simplex.begin(), simplex.end());
    std::vector<Index> faces = {0, 1, 2, 0, 3, 1, 0, 2, 3, 1, 3, 2};

    // list: vec4(normal, distance), index: min distance
    auto [normals, minFace] = getFaceNormals(polytope, faces);

    contact.depth = std::numeric_limits<float>::max();

    while (contact.depth == std::numeric_limits<float>::max()) {
        contact.normal = normals[minFace];
        contact.depth = normals[minFace].w;

        glm::vec3 support = minkowskiSupportPoint(body1, transform1, body2, transform2, contact.normal);
        const float curDepth = glm::dot(contact.normal, support);

        if (glm::abs(curDepth - contact.depth) > 1e-4f) {
            contact.depth = std::numeric_limits<float>::max();

            std::vector<std::pair<Index, Index>> uniqueEdges;

            for (std::size_t i = 0; i < normals.size(); i++) {
                if (isFacing(normals[i], support - polytope[faces[i * 3]])) {
                    std::size_t f = i * 3;

                    AddIfUniqueEdge(uniqueEdges, faces, f, f + 1);
                    AddIfUniqueEdge(uniqueEdges, faces, f + 1, f + 2);
                    AddIfUniqueEdge(uniqueEdges, faces, f + 2, f);

                    faces[f + 2] = faces.back();
                    faces.pop_back();

                    faces[f + 1] = faces.back();
                    faces.pop_back();

                    faces[f] = faces.back();
                    faces.pop_back();

                    normals[i] = normals.back(); // pop-erase
                    normals.pop_back();

                    i--;
                }
            }

            std::vector<Index> newFaces;
            for (auto [edgeIndex1, edgeIndex2] : uniqueEdges) {
                newFaces.push_back(edgeIndex1);
                newFaces.push_back(edgeIndex2);
                newFaces.push_back(polytope.size());
            }

            polytope.push_back(support);

            auto [newNormals, newMinFace] = getFaceNormals(polytope, newFaces);
            float oldMinDistance = std::numeric_limits<float>::max();
            for (std::size_t i = 0; i < normals.size(); i++) {
                if (normals[i].w < oldMinDistance) {
                    oldMinDistance = normals[i].w;
                    minFace = i;
                }
            }

            if (newNormals[newMinFace].w < oldMinDistance) {
                minFace = newMinFace + normals.size();
            }

            faces.insert(faces.end(), newFaces.begin(), newFaces.end());
            normals.insert(normals.end(), newNormals.begin(), newNormals.end());
        }
    }

    contact.depth += 1e-4f;

    contact.point1 = supportPoint(body1, transform1, contact.normal);
    contact.point2 = supportPoint(body2, transform2, -contact.normal);
}

glm::vec3 Gjk3D::supportPoint(const RigidBodyComponent &body, const Transform &transform, const glm::vec3 &direction) {
    const glm::mat3 modelNoTranslation = glm::mat3(transform.getModel());
    return transform.toWorld(body.calcSupportPoint(glm::transpose(modelNoTranslation) * direction));
}

glm::vec3 Gjk3D::minkowskiSupportPoint(const RigidBodyComponent &body1, const Transform &transform1, const RigidBodyComponent &body2, const Transform &transform2, const glm::vec3 &direction) {
    return supportPoint(body1, transform1, direction) - supportPoint(body2, transform2, -direction);
}

std::pair<std::vector<glm::vec4>, std::size_t> Gjk3D::getFaceNormals(const std::vector<glm::vec3> &polytope, const std::vector<Index> &faces) {
    std::vector<glm::vec4> normals;
    std::size_t minTriangle = 0;
    float minDistance = std::numeric_limits<float>::max();

    for (std::size_t i = 0; i < faces.size(); i += 3) {
        const glm::vec3 &a = polytope[faces[i]];
        const glm::vec3 &b = polytope[faces[i + 1]];
        const glm::vec3 &c = polytope[faces[i + 2]];

        glm::vec3 normal = glm::normalize(glm::cross(b - a, c - a));
        float distance = glm::dot(normal, a);

        if (distance < 0) {
            normal *= -1;
            distance *= -1;
        }

        normals.emplace_back(normal, distance);

        if (distance < minDistance) {
            minTriangle = i / 3;
            minDistance = distance;
        }
    }

    return {normals, minTriangle};
}

void Gjk3D::AddIfUniqueEdge(std::vector<std::pair<Index, Index>> &edges, const std::vector<Index> &faces, std::size_t a, std::size_t b) {
    auto reverse = std::find(edges.begin(), edges.end(), std::make_pair(faces[b], faces[a]));

    if (reverse != edges.end()) {
        edges.erase(reverse);
    } else {
        edges.emplace_back(faces[a], faces[b]);
    }
}

bool Gjk3D::isFacing(const glm::vec3 &vec, const glm::vec3 &direction) {
    return glm::dot(vec, direction) > 0.0f;
}

} // namespace Physbuzz
