#include "circle.hpp"

#include <glm/ext/scalar_constants.hpp>
#include <physbuzz/collision.hpp>
#include <physbuzz/renderer.hpp>
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
    if (info.isRenderable) {
        constexpr float MAX_VERTICES = 50;
        constexpr const float angleIncrement = (2.0f * glm::pi<float>()) / MAX_VERTICES;

        Physbuzz::Mesh mesh;

        // calc positions
        for (int i = 0; i < MAX_VERTICES; i++) {
            float angle = i * angleIncrement;
            mesh.positions.emplace_back(info.circle.radius * glm::vec3(glm::cos(angle), glm::sin(angle), 0.0f));
        }

        // calc indices
        for (int i = 1; i < MAX_VERTICES - 1; i++) {
            mesh.indices.push_back(glm::uvec3(0, i, i + 1));
        }
        mesh.indices.push_back(glm::uvec3(0, MAX_VERTICES - 1, 1));

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
        Physbuzz::ShaderPipeline shader = Physbuzz::ShaderPipeline(circleVertex, circleFrag);
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
        // Mx = (r*sin(theta))**2 * r).integrate((theta, 0, 2*pi)).integrate((r, 0, a)) * rho
        // My = (r*cos(theta))**2 * r).integrate((theta, 0, 2*pi)).integrate((r, 0, a)) * rho
        info.body.angular.inertia = info.body.mass * glm::pow(info.circle.radius, 2) / 2.0f;

        object.setComponent(info.body);
    }

    // create a rebuild callback
    {
        RebuildableComponent rebuilder = {
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

        object.setComponent(rebuilder);
    }

    return object.getId();
}
