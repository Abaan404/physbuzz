#include "circle.hpp"

#include <glm/ext/scalar_constants.hpp>
#include <physbuzz/collision.hpp>

template <>
Physbuzz::ObjectID ObjectBuilder::create(Physbuzz::Object &object, Circle &info) {
    // user-defined components
    object.setComponent(info.circle);
    object.setComponent(info.transform);
    object.setComponent(info.identifier);
    object.setComponent(info.resources);

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
        object.setComponent(mesh);

        // generate bounding box
        if (info.isCollidable) {
            object.setComponent(aabb);
        }
    }

    // build inertia
    {
        // Mx = (r*sin(theta))**2 * r).integrate((theta, 0, 2*pi)).integrate((r, 0, a)) * rho
        // My = (r*cos(theta))**2 * r).integrate((theta, 0, 2*pi)).integrate((r, 0, a)) * rho
        info.body.angular.inertia = info.body.mass * glm::pow(info.circle.radius, 2) / 2.0f;

        object.setComponent(info.body);
    }

    // create a rebuild callback
    {
        RebuildableComponent rebuilder = {
            .rebuild = [](ObjectBuilder &builder, Physbuzz::Object &object) {
                if (object.hasComponent<Physbuzz::Mesh>()) {
                    object.getComponent<Physbuzz::Mesh>().destroy();
                }

                Circle info = {
                    .body = object.getComponent<Physbuzz::RigidBodyComponent>(),
                    .transform = object.getComponent<Physbuzz::TransformableComponent>(),
                    .circle = object.getComponent<CircleComponent>(),
                    .identifier = object.getComponent<IdentifiableComponent>(),
                    .isCollidable = object.hasComponent<Physbuzz::AABBComponent>(),
                    .isRenderable = object.hasComponent<Physbuzz::Mesh>(),
                };

                object.eraseComponents();
                builder.create(object, info);
            },
        };

        object.setComponent(rebuilder);
    }

    return object.getId();
}
