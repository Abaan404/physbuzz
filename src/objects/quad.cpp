#include "quad.hpp"

#include <physbuzz/collision.hpp>
#include <physbuzz/renderer.hpp>
#include <physbuzz/shaders.hpp>

#include "shaders/quad.frag"
#include "shaders/quad.vert"

template <>
Physbuzz::ObjectID ObjectBuilder::create(Physbuzz::Object &object, Quad &info) {
    // user-defined components
    object.setComponent(info.quad);
    object.setComponent(info.transform);
    object.setComponent(info.identifier);

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
        Physbuzz::BoundingComponent bounding = Physbuzz::BoundingComponent(mesh);
        generate2DTexCoords(bounding, mesh);
        generate2DNormals(mesh);

        // apply transformations
        applyTransformsToMesh(info.transform, mesh);

        // setup rendering
        Physbuzz::Texture texture = m_Textures.getTexture("quad");
        Physbuzz::ShaderPipeline shader = Physbuzz::ShaderPipeline(quadVertex, quadFrag);
        Physbuzz::RenderComponent render = Physbuzz::RenderComponent(mesh, shader, texture);
        render.build();

        object.setComponent(render);

        // generate bounding box
        if (info.isCollidable) {
            bounding.aabb.max += info.transform.position;
            bounding.aabb.min += info.transform.position;

            object.setComponent(bounding);
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
                if (object.hasComponent<Physbuzz::RenderComponent>()) {
                    object.getComponent<Physbuzz::RenderComponent>().destroy();
                }

                Quad info = {
                    .body = object.getComponent<Physbuzz::RigidBodyComponent>(),
                    .transform = object.getComponent<Physbuzz::TransformableComponent>(),
                    .quad = object.getComponent<QuadComponent>(),
                    .identifier = object.getComponent<IdentifiableComponent>(),
                    .isCollidable = object.hasComponent<Physbuzz::BoundingComponent>(),
                    .isRenderable = object.hasComponent<Physbuzz::RenderComponent>(),
                };

                object.eraseComponents();
                builder.create(object, info);
            },
        };

        object.setComponent(rebuilder);
    }

    return object.getId();
}
