#include "circle.hpp"

#include <glm/ext/scalar_constants.hpp>
#include <physbuzz/physics/collision.hpp>

template <>
Physbuzz::ObjectID ObjectBuilder::create(Physbuzz::ObjectID object, Circle &info) {
    // user-defined components
    scene->setComponent(object, info.circle, info.transform, info.identifier, info.resources);

    // generate mesh
    if (info.isRenderable) {
        constexpr float MAX_VERTICES = 50;
        constexpr const float angleIncrement = (2.0f * glm::pi<float>()) / MAX_VERTICES;

        Physbuzz::Mesh mesh;
        mesh.vertices.resize(MAX_VERTICES);

        // calc positions
        for (int i = 0; i < MAX_VERTICES; i++) {
            float angle = i * angleIncrement;
            mesh.vertices[i].position = info.circle.radius * glm::vec3(glm::cos(angle), glm::sin(angle), 0.0f);
        }

        // calc indices
        for (int i = 1; i < MAX_VERTICES - 1; i++) {
            mesh.indices.push_back(glm::uvec3(0, i, i + 1));
        }
        mesh.indices.push_back(glm::uvec3(0, MAX_VERTICES - 1, 1));

        // calc vertices
        Physbuzz::AABBComponent aabb = Physbuzz::AABBComponent(mesh, info.transform);
        generate2DTexCoords(aabb, mesh);
        generate2DNormals(mesh);

        mesh.build();
        scene->setComponent(object, mesh);

        // generate bounding box
        if (info.isCollidable) {
            scene->setComponent(object, aabb);
        }
    }

    // build inertia
    {
        // Mx = (r*sin(theta))**2 * r).integrate((theta, 0, 2*pi)).integrate((r, 0, a)) * rho
        // My = (r*cos(theta))**2 * r).integrate((theta, 0, 2*pi)).integrate((r, 0, a)) * rho
        info.body.angular.inertia = info.body.mass * glm::pow(info.circle.radius, 2) / 2.0f;

        scene->setComponent(object, info.body);
    }

    // create a rebuild callback
    {
        RebuildableComponent rebuilder = {
            .rebuild = [](ObjectBuilder &builder, Physbuzz::ObjectID object) {
                Circle info = {
                    .body = builder.scene->getComponent<Physbuzz::RigidBodyComponent>(object),
                    .transform = builder.scene->getComponent<Physbuzz::TransformableComponent>(object),
                    .circle = builder.scene->getComponent<CircleComponent>(object),
                    .identifier = builder.scene->getComponent<IdentifiableComponent>(object),
                    .isCollidable = builder.scene->containsComponent<Physbuzz::AABBComponent>(object),
                    .isRenderable = builder.scene->containsComponent<Physbuzz::Mesh>(object),
                };

                builder.create(object, info);
            },
        };

        scene->setComponent(object, rebuilder);
    }

    return object;
}
