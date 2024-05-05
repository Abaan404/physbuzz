#include "collision.hpp"

#include "../dynamics/dynamics.hpp"
#include "../objects/objects.hpp"
#include <iostream>
#include <physbuzz/renderer.hpp>
#include <vector>

// sweep and prune
void Collision::tick(Physbuzz::Scene &scene) {

    // classic O(n^2)
    std::vector<Physbuzz::Object> &objects = scene.getObjects();
    for (auto &object1 : objects) {
        for (auto &object2 : objects) {
            if (&object1 == &object2) {
                continue;
            }

            if (check(object1, object2)) {
                std::cout
                    << "Collision"
                    << std::endl;

                resolve(object1, object2);
            }
        }
    }
}

bool Collision::check(Physbuzz::Object &object1, Physbuzz::Object &object2) {
    // only handle collision between circles for now
    if (!(object1.hasComponent<RadiusComponent>() && object2.hasComponent<RadiusComponent>())) {
        return false;
    }

    RadiusComponent &radius1 = object1.getComponent<RadiusComponent>();
    RadiusComponent &radius2 = object2.getComponent<RadiusComponent>();
    TransformableComponent &transform1 = object1.getComponent<TransformableComponent>();
    TransformableComponent &transform2 = object2.getComponent<TransformableComponent>();

    const float dist_squared = (transform2.position.x - transform1.position.x) * (transform2.position.x - transform1.position.x) +
                               (transform2.position.y - transform1.position.y) * (transform2.position.y - transform1.position.y);

    const float r_squared = (radius1.radius + radius2.radius) * (radius1.radius + radius2.radius);

    return dist_squared <= r_squared;
}

void Collision::resolve(Physbuzz::Object &object1, Physbuzz::Object &object2) {
    // only handle collision between circles for now
    if (!(object1.hasComponent<RadiusComponent>() && object2.hasComponent<RadiusComponent>())) {
        return;
    }

    RadiusComponent &radius1 = object1.getComponent<RadiusComponent>();
    RadiusComponent &radius2 = object2.getComponent<RadiusComponent>();
    TransformableComponent &transform1 = object1.getComponent<TransformableComponent>();
    TransformableComponent &transform2 = object2.getComponent<TransformableComponent>();
    RigidBodyComponent &body1 = object1.getComponent<RigidBodyComponent>();
    RigidBodyComponent &body2 = object2.getComponent<RigidBodyComponent>();

    glm::vec3 normal = glm::normalize(transform2.position - transform1.position);
    glm::vec3 tangent = glm::cross(normal, glm::vec3(0, 0, 1));

    float overlap = (radius1.radius + radius2.radius) - glm::length(transform2.position - transform1.position);
    glm::vec3 displacement = 0.5f * overlap * normal;

    transform1.position -= displacement;
    transform2.position += displacement;

    glm::vec3 rel_velocity = body2.velocity - body1.velocity;

    float normal_impulse = glm::dot(rel_velocity, normal) * (body1.mass + body2.mass);
    float tangent_impulse = glm::dot(rel_velocity, tangent) * (body1.mass + body2.mass);
    float mass_total = body1.mass + body2.mass;

    body1.velocity += (normal_impulse * normal + tangent_impulse * tangent) / mass_total;
    body2.velocity -= (normal_impulse * normal + tangent_impulse * tangent) / mass_total;

    object1.getComponent<Physbuzz::MeshComponent>().redraw();
    object2.getComponent<Physbuzz::MeshComponent>().redraw();

    glm::vec3 position1 = transform1.position;
    glm::vec3 position2 = transform2.position;

    float radius_1 = radius1.radius;
    float radius_2 = radius2.radius;
}
