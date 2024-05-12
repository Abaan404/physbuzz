#include "circle.hpp"

#include "../collision/collision.hpp"
#include <glm/ext/scalar_constants.hpp>
#include <physbuzz/mesh.hpp>
#include <physbuzz/shaders.hpp>

#include "shaders/circle.frag"
#include "shaders/circle.vert"

template <>
Physbuzz::ObjectID ObjectBuilder<CircleInfo>::build(Physbuzz::Object &object, CircleInfo &info) {
    // user-defined components
    object.setComponent(info.circle);
    object.setComponent(info.transform);
    object.setComponent(info.body);
    object.setComponent(info.identifier);

    // generate mesh
    Physbuzz::MeshComponent mesh;
    {
        constexpr float MAX_VERTICES = 50;
        constexpr const float angleIncrement = (2.0f * glm::pi<float>()) / MAX_VERTICES;

        // good ol trig
        for (int i = 0; i < MAX_VERTICES; i++) {
            float angle = i * angleIncrement;
            mesh.screenVertices.push_back(info.circle.radius * glm::vec3(glm::cos(angle), glm::sin(angle), 0.0f));
        }

        for (int i = 1; i < MAX_VERTICES - 1; i++) {
            mesh.indices.push_back(glm::uvec3(0, i, i + 1));
        }
        mesh.indices.push_back(glm::uvec3(0, MAX_VERTICES - 1, 1));

        mesh.orignalVertices = mesh.screenVertices;
        for (auto &vertex : mesh.screenVertices) {
            vertex = info.transform.orientation * vertex + info.transform.position;
        }

        object.setComponent(mesh);
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
        BoundingComponent component = BoundingComponent(mesh);
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
                    .transform = object.getComponent<TransformableComponent>(),
                    .body = object.getComponent<RigidBodyComponent>(),
                    .circle = object.getComponent<CircleComponent>(),
                    .identifier = object.getComponent<IdentifiableComponent>(),
                    .isCollidable = object.hasComponent<BoundingComponent>(),
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
