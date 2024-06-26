#include "quad.hpp"

#include <physbuzz/collision.hpp>
#include <physbuzz/mesh.hpp>
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
    {
        glm::vec3 min = glm::vec3(-info.quad.width / 2.0f, -info.quad.height / 2.0f, 0.0f);
        glm::vec3 max = glm::vec3(info.quad.width / 2.0f, info.quad.height / 2.0f, 0.0f);

        std::vector<glm::uvec3> indices = {
            {0, 1, 2},
            {0, 3, 2},
        };

        std::vector<glm::vec3> vertices = {
            {min.x, min.y, 0.0f}, // top-left
            {min.x, max.y, 0.0f}, // top-right
            {max.x, max.y, 0.0f}, // bottom-right
            {max.x, min.y, 0.0f}, // bottom-left
        };

        Physbuzz::MeshComponent mesh = Physbuzz::MeshComponent(indices, vertices);

        // apply transformations
        for (auto &vertex : mesh.vertices) {
            vertex = info.transform.orientation * vertex + info.transform.position;
        }

        for (auto &normal : mesh.normals) {
            normal = info.transform.orientation * normal;
        }

        object.setComponent(mesh);
    }

    // build inertia
    {
        // Mx = (y**2).integrate((x, -a/2, a/2)).integrate((y, -b/2, b/2)) * rho
        // My = (x**2).integrate((x, -a/2, a/2)).integrate((y, -b/2, b/2)) * rho
        info.body.angular.inertia = info.body.mass * (glm::pow(info.quad.width, 2) + glm::pow(info.quad.height, 2)) / 12.0f;

        object.setComponent(info.body);
    }

    // build gl context
    if (info.isRenderable) {
        Physbuzz::ShaderContext shader = Physbuzz::ShaderContext(quadVertex, quadFrag);
        shader.build();
        GLuint program = shader.load();

        Physbuzz::RenderComponent component = Physbuzz::RenderComponent();
        component.build();
        component.setProgram(program);

        object.setComponent(component);
    }

    // generate bounding box
    if (info.isCollidable) {
        Physbuzz::MeshComponent &mesh = object.getComponent<Physbuzz::MeshComponent>();
        Physbuzz::BoundingComponent component = Physbuzz::BoundingComponent(mesh);

        object.setComponent(component);
    }

    // create a rebuild callback
    {
        RebuildableComponent component = {
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

        object.setComponent(component);
    }

    return object.getId();
}
