#include "collision.hpp"

#include "../dynamics/dynamics.hpp"
#include "../objects/objects.hpp"
#include <physbuzz/renderer.hpp>
#include <vector>

// sweep and prune
void Collision::tick(Physbuzz::Scene &scene) {
    scene.sortObjects([](Physbuzz::Object &object1, Physbuzz::Object &object2) {
        AABBComponent &aabb1 = object1.getComponent<AABBComponent>();
        AABBComponent &aabb2 = object2.getComponent<AABBComponent>();

        return aabb1.min.x < aabb2.min.x;
    });

    std::vector<Physbuzz::Object> &objects = scene.getObjects();
    for (auto &object1 : objects) {
        for (auto &object2 : objects) {
            if (&object1 == &object2) {
                continue;
            }

            AABBComponent &aabb1 = object1.getComponent<AABBComponent>();
            AABBComponent &aabb2 = object2.getComponent<AABBComponent>();

            if (aabb2.min.x > aabb1.max.x) {
                continue;
            }

            if (check(object1, object2)) {
                resolve(object1, object2);
            }
        }
    }
}

// hardcoded collision checks and resolvers
bool Collision::check(Physbuzz::Object &object1, Physbuzz::Object &object2) {
    if (object1.hasComponent<CircleComponent>() && object2.hasComponent<CircleComponent>()) {
        CircleComponent &circle1 = object1.getComponent<CircleComponent>();
        CircleComponent &circle2 = object2.getComponent<CircleComponent>();
        TransformableComponent &transform1 = object1.getComponent<TransformableComponent>();
        TransformableComponent &transform2 = object2.getComponent<TransformableComponent>();

        const float dist_squared = (transform2.position.x - transform1.position.x) * (transform2.position.x - transform1.position.x) +
                                   (transform2.position.y - transform1.position.y) * (transform2.position.y - transform1.position.y);

        const float r_squared = (circle1.radius + circle2.radius) * (circle1.radius + circle2.radius);

        return dist_squared <= r_squared;

    } else if (object1.hasComponent<QuadComponent>() && object2.hasComponent<QuadComponent>()) {
        QuadComponent &quad1 = object1.getComponent<QuadComponent>();
        QuadComponent &quad2 = object2.getComponent<QuadComponent>();

        TransformableComponent &transform1 = object1.getComponent<TransformableComponent>();
        TransformableComponent &transform2 = object2.getComponent<TransformableComponent>();

        glm::vec3 min1 = transform1.position - glm::vec3(quad1.width / 2.0f, quad1.height / 2.0f, 0.0f);
        glm::vec3 min2 = transform2.position - glm::vec3(quad2.width / 2.0f, quad2.height / 2.0f, 0.0f);

        glm::vec3 max1 = min1 + glm::vec3(quad1.width, quad1.height, 0.0f);
        glm::vec3 max2 = min2 + glm::vec3(quad2.width, quad2.height, 0.0f);

        bool intersectX = max1.x > min2.x && min1.x < max2.x;
        bool intersectY = max1.y > min2.y && min1.y < max2.y;

        return intersectX && intersectY;

    } else if (object1.hasComponent<QuadComponent>() && object2.hasComponent<CircleComponent>()) {
        QuadComponent &quad1 = object1.getComponent<QuadComponent>();
        CircleComponent &circle2 = object2.getComponent<CircleComponent>();

        TransformableComponent &transform1 = object1.getComponent<TransformableComponent>();
        TransformableComponent &transform2 = object2.getComponent<TransformableComponent>();

        glm::vec3 min = transform1.position - glm::vec3(quad1.width / 2.0f, quad1.height / 2.0f, 0.0f);
        glm::vec3 max = transform1.position + glm::vec3(quad1.width / 2.0f, quad1.height / 2.0f, 0.0f);

        glm::vec3 point = glm::clamp(transform2.position, min, max);
        glm::vec3 dist = transform2.position - point;

        float dist_squared = glm::dot(dist, dist);
        float r_squared = circle2.radius * circle2.radius;

        return dist_squared <= r_squared;
    }

    return false;
}

// hardcoded collision resolvers
void Collision::resolve(Physbuzz::Object &object1, Physbuzz::Object &object2) {
    if (!(object1.hasComponent<RigidBodyComponent>() && object2.hasComponent<RigidBodyComponent>())) {
        return;
    }

    if (!(object1.hasComponent<TransformableComponent>() && object2.hasComponent<TransformableComponent>())) {
        return;
    }

    RigidBodyComponent &body1 = object1.getComponent<RigidBodyComponent>();
    RigidBodyComponent &body2 = object2.getComponent<RigidBodyComponent>();

    TransformableComponent &transform1 = object1.getComponent<TransformableComponent>();
    TransformableComponent &transform2 = object2.getComponent<TransformableComponent>();

    AABBComponent &bounding1 = object1.getComponent<AABBComponent>();
    AABBComponent &bounding2 = object2.getComponent<AABBComponent>();

    DisplacementOutput resultDisplacement = calcDisplacement(object1, object2);
    ImpulseOutput resultImpulse = calcImpulse(object1, object2, resultDisplacement);

    glm::vec3 deltaPosition = resultDisplacement.normal * resultDisplacement.depth * 0.5f;
    glm::vec3 deltaVelocity = (resultImpulse.normal + resultImpulse.tangent) / resultImpulse.mass;

    transform1.position -= deltaPosition;
    bounding1.min -= deltaPosition;
    bounding1.max -= deltaPosition;
    body1.velocity += deltaVelocity;

    transform2.position += deltaPosition;
    bounding2.min += deltaPosition;
    bounding2.max += deltaPosition;
    body2.velocity -= deltaVelocity;

    if (object1.hasComponent<Physbuzz::MeshComponent>() && object2.hasComponent<Physbuzz::MeshComponent>()) {
        updateMesh(object1, object2, deltaPosition);
    }
}

DisplacementOutput Collision::calcDisplacement(Physbuzz::Object &object1, Physbuzz::Object &object2) {
    TransformableComponent &transform1 = object1.getComponent<TransformableComponent>();
    TransformableComponent &transform2 = object2.getComponent<TransformableComponent>();

    DisplacementOutput out = {
        .normal = glm::vec3(0.0f, 0.0f, 0.0f),
        .tangent = glm::vec3(0.0f, 0.0f, 0.0f),
        .depth = 0.0f,
    };

    if (object1.hasComponent<CircleComponent>() && object2.hasComponent<CircleComponent>()) {
        CircleComponent &radius1 = object1.getComponent<CircleComponent>();
        CircleComponent &radius2 = object2.getComponent<CircleComponent>();
        float overlap = (radius1.radius + radius2.radius) - glm::length(transform2.position - transform1.position);

        out.depth = overlap;
        out.normal = glm::normalize(transform2.position - transform1.position);
        out.tangent = glm::cross(out.normal, glm::vec3(0, 0, 1));

    } else if (object1.hasComponent<QuadComponent>() && object2.hasComponent<QuadComponent>()) {
        QuadComponent &quad1 = object1.getComponent<QuadComponent>();
        QuadComponent &quad2 = object2.getComponent<QuadComponent>();

        TransformableComponent &transform1 = object1.getComponent<TransformableComponent>();
        TransformableComponent &transform2 = object2.getComponent<TransformableComponent>();

        glm::vec3 min1 = transform1.position - glm::vec3(quad1.width / 2.0f, quad1.height / 2.0f, 0.0f);
        glm::vec3 min2 = transform2.position - glm::vec3(quad2.width / 2.0f, quad2.height / 2.0f, 0.0f);

        glm::vec3 max1 = min1 + glm::vec3(quad1.width, quad1.height, 0.0f);
        glm::vec3 max2 = min2 + glm::vec3(quad2.width, quad2.height, 0.0f);

        // no rotation for now
        glm::vec3 closestPoint1 = glm::clamp(transform2.position, min1, max1);
        glm::vec3 closestPoint2 = glm::clamp(transform1.position, min2, max2);

        glm::vec3 distance = closestPoint1 - closestPoint2;

        out.depth = glm::length(distance);
        out.normal = glm::normalize(distance);
        out.tangent = glm::cross(out.normal, glm::vec3(0, 0, 1));

    } else if (object1.hasComponent<QuadComponent>() && object2.hasComponent<CircleComponent>()) {
        QuadComponent &quad1 = object1.getComponent<QuadComponent>();
        CircleComponent &circle2 = object2.getComponent<CircleComponent>();

        glm::vec3 min = transform1.position - glm::vec3(quad1.width / 2.0f, quad1.height / 2.0f, 0.0f);
        glm::vec3 max = transform1.position + glm::vec3(quad1.width / 2.0f, quad1.height / 2.0f, 0.0f);

        // no rotation for now
        glm::vec3 closestPoint = glm::clamp(transform2.position, min, max);
        glm::vec3 distance = transform2.position - closestPoint;
        float overlap = circle2.radius - glm::length(distance);

        out.depth = overlap;
        out.normal = glm::normalize(distance);
        out.tangent = glm::cross(out.normal, glm::vec3(0, 0, 1));
    }

    // this will surely be fine
    if (glm::any(glm::isnan(out.normal))) {
        out.normal = glm::vec3(0.0f, 1.0f, 0.0f);
        out.tangent = glm::cross(out.normal, glm::vec3(0, 0, 1));
    }

    return out;
}

ImpulseOutput Collision::calcImpulse(Physbuzz::Object &object1, Physbuzz::Object &object2, DisplacementOutput &displacement) {
    RigidBodyComponent &body1 = object1.getComponent<RigidBodyComponent>();
    RigidBodyComponent &body2 = object2.getComponent<RigidBodyComponent>();

    glm::vec3 relativeVelocity = body2.velocity - body1.velocity;

    return {
        .normal = glm::dot(relativeVelocity, displacement.normal) * (body1.mass + body2.mass) * displacement.normal,
        .tangent = glm::dot(relativeVelocity, displacement.tangent) * (body1.mass + body2.mass) * displacement.tangent,
        .mass = body1.mass + body2.mass,
    };
}

void Collision::updateMesh(Physbuzz::Object &object1, Physbuzz::Object &object2, glm::vec3 &delta) {
    Physbuzz::MeshComponent &mesh1 = object1.getComponent<Physbuzz::MeshComponent>();
    Physbuzz::MeshComponent &mesh2 = object2.getComponent<Physbuzz::MeshComponent>();

    std::vector<glm::vec3> vertex1 = mesh1.getVertex();
    std::vector<glm::vec3> vertex2 = mesh2.getVertex();

    for (auto &vertex : vertex1) {
        vertex -= delta;
    }

    for (auto &vertex : vertex2) {
        vertex += delta;
    }

    mesh1.setVertex(vertex1);
    mesh2.setVertex(vertex2);
}
