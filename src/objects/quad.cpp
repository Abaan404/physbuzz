#include "quad.hpp"

#include <physbuzz/physics/collision.hpp>
#include <physbuzz/render/shaders.hpp>

template <>
Physbuzz::ObjectID ObjectBuilder::create(Physbuzz::ObjectID object, Quad &info) {
    // user-defined components
    scene->setComponent(object, info.quad, info.identifier, info.resources);

    // generate mesh
    if (info.isRenderable) {
        glm::vec3 min = glm::vec3(-info.quad.width / 2.0f, -info.quad.height / 2.0f, 0.0f);
        glm::vec3 max = glm::vec3(info.quad.width / 2.0f, info.quad.height / 2.0f, 0.0f);

        Physbuzz::MeshComponent mesh = Physbuzz::MeshComponent(info.model);
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
        generate2DTexCoords(mesh);
        generate2DNormals(mesh);

        mesh.build();
        scene->setComponent(object, mesh);

        // generate bounding box
        if (info.isCollidable) {
            Physbuzz::AABBComponent aabb = Physbuzz::AABBComponent(mesh);
            scene->setComponent(object, aabb);
        }
    }

    // build inertia
    {
        // Mx = (y**2).integrate((x, -a/2, a/2)).integrate((y, -b/2, b/2)) * rho
        // My = (x**2).integrate((x, -a/2, a/2)).integrate((y, -b/2, b/2)) * rho
        info.body.angular.inertia = info.body.mass * (glm::pow(info.quad.width, 2) + glm::pow(info.quad.height, 2)) / 12.0f;

        scene->setComponent(object, info.body);
    }

    // create a rebuild callback
    {
        RebuildableComponent rebuilder = {
            .rebuild = [](ObjectBuilder &builder, Physbuzz::ObjectID object) {
                Quad info = {
                    .body = builder.scene->getComponent<Physbuzz::RigidBodyComponent>(object),
                    .quad = builder.scene->getComponent<QuadComponent>(object),
                    .identifier = builder.scene->getComponent<IdentifiableComponent>(object),
                    .isCollidable = builder.scene->containsComponent<Physbuzz::AABBComponent>(object),
                    .isRenderable = false,
                };

                if (builder.scene->containsComponent<Physbuzz::MeshComponent>(object)) {
                    info.isRenderable = true;
                    info.model = builder.scene->getComponent<Physbuzz::MeshComponent>(object).model;
                }

                builder.create(object, info);
            },
        };

        scene->setComponent(object, rebuilder);
    }

    return object;
}
