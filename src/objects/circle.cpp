#include "circle.hpp"

#include <glm/ext/scalar_constants.hpp>
#include <physbuzz/collision.hpp>
#include <physbuzz/mesh.hpp>
#include <physbuzz/shaders.hpp>

#include "shaders/circle.frag"
#include "shaders/circle.vert"

template <>
Physbuzz::ObjectID ObjectBuilder<CircleInfo>::build(Physbuzz::Object &object, CircleInfo &info) {
    // user-defined components
    object.setComponent(info.circle);
    object.setComponent(info.transform);
    object.setComponent(info.identifier);

    // generate mesh
    {
        constexpr float MAX_VERTICES = 50;
        constexpr const float angleIncrement = (2.0f * glm::pi<float>()) / MAX_VERTICES;

        std::vector<glm::uvec3> indices;
        std::vector<glm::vec3> vertices;

        // good ol trig
        for (int i = 0; i < MAX_VERTICES; i++) {
            float angle = i * angleIncrement;
            vertices.push_back(info.circle.radius * glm::vec3(glm::cos(angle), glm::sin(angle), 0.0f));
        }

        for (int i = 1; i < MAX_VERTICES - 1; i++) {
            indices.push_back(glm::uvec3(0, i, i + 1));
        }
        indices.push_back(glm::uvec3(0, MAX_VERTICES - 1, 1));

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
        // Mx = (r*sin(theta))**2 * r).integrate((theta, 0, 2*pi)).integrate((r, 0, a)) * rho
        // My = (r*cos(theta))**2 * r).integrate((theta, 0, 2*pi)).integrate((r, 0, a)) * rho
        info.body.angular.inertia = info.body.mass * glm::pow(info.circle.radius, 2) / 2.0f;

        object.setComponent(info.body);
    }

    // build gl context
    if (info.isRenderable) {
        static Physbuzz::ShaderContext shader = Physbuzz::ShaderContext(circleVertex, circleFrag);
        static GLuint program = shader.load();

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

                CircleInfo info = {
                    .body = object.getComponent<Physbuzz::RigidBodyComponent>(),
                    .transform = object.getComponent<Physbuzz::TransformableComponent>(),
                    .circle = object.getComponent<CircleComponent>(),
                    .identifier = object.getComponent<IdentifiableComponent>(),
                    .isCollidable = object.hasComponent<Physbuzz::BoundingComponent>(),
                    .isRenderable = object.hasComponent<Physbuzz::RenderComponent>(),
                };

                object.eraseComponents();
                ObjectBuilder<CircleInfo>::build(object, info);
            },
        };

        object.setComponent(component);
    }

    return object.getId();
}
