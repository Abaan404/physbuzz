#include "quad.hpp"

#include <physbuzz/collision.hpp>
#include <physbuzz/renderer.hpp>
#include <physbuzz/shaders.hpp>

#include "shaders/quad.frag"
#include "shaders/quad.vert"

template <>
Physbuzz::ObjectID ObjectBuilder<QuadInfo>::build(Physbuzz::Object &object, QuadInfo &info) {
    // user-defined components
    object.setComponent(info.quad);
    object.setComponent(info.transform);
    object.setComponent(info.identifier);

    // generate mesh
    if (info.isRenderable) {
        glm::vec3 min = glm::vec3(-info.quad.width / 2.0f, -info.quad.height / 2.0f, 0.0f);
        glm::vec3 max = glm::vec3(info.quad.width / 2.0f, info.quad.height / 2.0f, 0.0f);

        Physbuzz::Mesh mesh;

        // calc positions
        mesh.positions = {
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

        // calc normals
        mesh.vertices.resize(mesh.positions.size());
        for (std::size_t i = 0; i < mesh.positions.size(); ++i) {
            const std::size_t next = (i + 1) % mesh.positions.size();                  // cycle next vertex
            const glm::vec3 tangent = mesh.positions[next] - mesh.positions[i];        // get the tangent
            const glm::vec3 normal = glm::cross(tangent, glm::vec3(0.0f, 0.0f, 1.0f)); // cross prod for normal

            mesh.vertices[i].normal = glm::normalize(normal);
        }

        // apply transformations
        for (auto &vertex : mesh.positions) {
            vertex = info.transform.orientation * vertex + info.transform.position;
        }

        for (auto &vertex : mesh.vertices) {
            vertex.normal = info.transform.orientation * vertex.normal;
        }

        // setup rendering
        Physbuzz::ShaderPipeline shader = Physbuzz::ShaderPipeline(quadVertex, quadFrag);
        Physbuzz::RenderComponent render = Physbuzz::RenderComponent(mesh, shader);
        render.build();

        object.setComponent(render);

        // generate bounding box
        if (info.isCollidable) {
            Physbuzz::BoundingComponent bounding = Physbuzz::BoundingComponent(mesh);
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
            .rebuild = [](Physbuzz::Object &object) {
                if (object.hasComponent<Physbuzz::RenderComponent>()) {
                    object.getComponent<Physbuzz::RenderComponent>().destroy();
                }

                QuadInfo info = {
                    .body = object.getComponent<Physbuzz::RigidBodyComponent>(),
                    .transform = object.getComponent<Physbuzz::TransformableComponent>(),
                    .quad = object.getComponent<QuadComponent>(),
                    .identifier = object.getComponent<IdentifiableComponent>(),
                    .isCollidable = object.hasComponent<Physbuzz::BoundingComponent>(),
                    .isRenderable = object.hasComponent<Physbuzz::RenderComponent>(),
                };

                object.eraseComponents();
                ObjectBuilder<QuadInfo>::build(object, info);
            },
        };

        object.setComponent(rebuilder);
    }

    return object.getId();
}
