#include "quad.hpp"

#include <physbuzz/collision.hpp>
#include <physbuzz/shaders.hpp>

template <>
Physbuzz::ObjectID ObjectBuilder::create(Physbuzz::Object &object, Quad &info) {
    // user-defined components
    object.setComponent(info.quad);
    object.setComponent(info.transform);
    object.setComponent(info.identifier);
    object.setComponent(info.resources);

    // generate mesh
    if (info.isRenderable) {
        glm::vec3 min = glm::vec3(-info.quad.width / 2.0f, -info.quad.height / 2.0f, 0.0f);
        glm::vec3 max = glm::vec3(info.quad.width / 2.0f, info.quad.height / 2.0f, 0.0f);

        Physbuzz::Mesh mesh;
        mesh.vertices.resize(4);

        // calc positions
        std::vector<glm::vec3> positions = {
            {min.x, min.y, 0.0f}, // top-left
            {min.x, max.y, 0.0f}, // top-right
            {max.x, max.y, 0.0f}, // bottom-right
            {max.x, min.y, 0.0f}, // bottom-left
        };

        // calc indices
        mesh.indices = {
            {0, 1, 2},
            {0, 3, 2},
        };

        for (std::size_t i = 0; i < mesh.vertices.size(); i++) {
            mesh.vertices[i].position = positions[i];
        }

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
        // Mx = (y**2).integrate((x, -a/2, a/2)).integrate((y, -b/2, b/2)) * rho
        // My = (x**2).integrate((x, -a/2, a/2)).integrate((y, -b/2, b/2)) * rho
        info.body.angular.inertia = info.body.mass * (glm::pow(info.quad.width, 2) + glm::pow(info.quad.height, 2)) / 12.0f;

        object.setComponent(info.body);
    }

    // create a rebuild callback
    {
        RebuildableComponent rebuilder = {
            .rebuild = [](ObjectBuilder &builder, Physbuzz::Object &object) {
                if (object.hasComponent<Physbuzz::Mesh>()) {
                    object.getComponent<Physbuzz::Mesh>().destroy();
                }

                Quad info = {
                    .body = object.getComponent<Physbuzz::RigidBodyComponent>(),
                    .transform = object.getComponent<Physbuzz::TransformableComponent>(),
                    .quad = object.getComponent<QuadComponent>(),
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
