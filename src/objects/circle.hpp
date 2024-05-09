#pragma once

#include "shaders/circle.frag"
#include "shaders/circle.vert"

#include "../collision/collision.hpp"
#include "../dynamics/dynamics.hpp"
#include "objects.hpp"
#include <glm/ext/scalar_constants.hpp>
#include <physbuzz/mesh.hpp>
#include <physbuzz/shaders.hpp>

struct CircleComponent {
    float radius = 0.0f;
};

struct CircleInfo {
    // physics info
    TransformableComponent transform;
    RigidBodyComponent body;

    // geometry
    CircleComponent circle;

    // naming
    IdentifiableComponent identifier = {
        .type = ObjectType::Circle,
        .name = "Circle",
    };

    bool isCollidable = false;
    bool isRenderable = false;
};

template <>
inline Physbuzz::ObjectID ObjectBuilder<CircleInfo>::build(Physbuzz::Object &object, CircleInfo &info) {
    // user-defined components
    object.setComponent(info.circle);
    object.setComponent(info.transform);
    object.setComponent(info.body);
    object.setComponent(info.identifier);

    // adhoc means to generate vertices for a quad without polluting the namespace
    auto generateVertices = [&]() {
        struct Mesh {
            std::vector<glm::vec3> vertices;
            std::vector<glm::uvec3> indices;
        } mesh;

        constexpr float MAX_VERTICES = 50;
        constexpr const float angleIncrement = (2.0f * glm::pi<float>()) / MAX_VERTICES;

        // good ol trig
        for (int i = 0; i < MAX_VERTICES; i++) {
            float angle = i * angleIncrement;
            float x = info.transform.position.x + info.circle.radius * glm::cos(angle);
            float y = info.transform.position.y + info.circle.radius * glm::sin(angle);
            mesh.vertices.push_back(glm::vec3(x, y, 0));
        }

        for (int i = 1; i < MAX_VERTICES - 1; i++) {
            mesh.indices.push_back(glm::uvec3(0, i, i + 1));
        }
        mesh.indices.push_back(glm::uvec3(0, MAX_VERTICES - 1, 1));

        return mesh;
    };

    // generate mesh
    if (info.isRenderable) {
        Physbuzz::MeshComponent mesh = Physbuzz::MeshComponent();
        static Physbuzz::ShaderContext shader = Physbuzz::ShaderContext(circleVertex, circleFrag);
        static GLuint program = shader.load();
        auto meshes = generateVertices();

        mesh.build();
        mesh.setProgram(program);
        mesh.setVertex(meshes.vertices);
        mesh.setIndex(meshes.indices);

        object.setComponent(mesh);
    }

    // generate bounding box
    if (info.isCollidable) {
        std::vector<glm::vec3> vertices;

        if (info.isRenderable) {
            vertices = object.getComponent<Physbuzz::MeshComponent>().getVertex();
        } else {
            vertices = generateVertices().vertices;
        }

        object.setComponent(AABBComponent::buildWithVertex(vertices));
    }

    return object.getId();
}
