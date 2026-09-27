#pragma once

#include "../ecs/defines.hpp"
#include "../ecs/scene.hpp"
#include <glm/glm.hpp>
#include <vector>

namespace Physbuzz {

struct RigidBodyComponent;

struct Contact {
    ObjectID object1;
    ObjectID object2;

    float depth = std::numeric_limits<float>::max();
    glm::vec3 normal = {0.0f, 0.0f, 1.0f};

    glm::vec3 point1 = {0.0f, 0.0f, 0.0f};
    glm::vec3 point2 = {0.0f, 0.0f, 0.0f};
};

template <typename T>
class CollisionDetector {
  public:
    CollisionDetector(Scene *scene)
        : m_Scene(scene) {}

    bool build() {
        return static_cast<T *>(this)->buildImpl();
    }

    bool destroy() {
        return static_cast<T *>(this)->destroyImpl();
    }

    bool check(Contact &contact) {
        return static_cast<T *>(this)->checkImpl(contact);
    }

    std::vector<Contact> findBroad() {
        return static_cast<T *>(this)->findBroadImpl();
    }

    void findNarrow(std::vector<Contact> &contacts) {
        static_cast<T *>(this)->findNarrowImpl(contacts);
    }

  protected:
    Scene *m_Scene;

    std::vector<Contact> findBroadImpl() {
        std::vector<Contact> contacts;

        for (const auto &object1 : m_Scene->getObjects()) {
            for (const auto &object2 : m_Scene->getObjects()) {
                if (object1 == object2) {
                    continue;
                }

                Contact contact = {
                    .object1 = object1,
                    .object2 = object2,
                };

                if (check(contact)) {
                    contacts.emplace_back(std::move(contact));
                }
            }
        }

        return contacts;
    }

    void findNarrowImpl(std::vector<Contact> &contacts) {
        std::erase_if(contacts, [this](Contact &c) {
            return !check(c);
        });
    }
};

template <typename T>
class CollisionResolver {
  public:
    CollisionResolver(Scene *scene)
        : m_Scene(scene) {}

    bool build() {
        return static_cast<T *>(this)->buildImpl();
    }

    bool destroy() {
        return static_cast<T *>(this)->destroyImpl();
    }

    void solve(const Contact &contact) {
        static_cast<T *>(this)->solveImpl(contact);
    }

    void solve(const std::vector<Contact> &contacts) {
        static_cast<T *>(this)->solveImpl(contacts);
    }

  protected:
    Scene *m_Scene;

    void solveImpl(const std::vector<Contact> &contacts) {
        for (const auto &contact : contacts) {
            solve(contact);
        }
    }
};

} // namespace Physbuzz
