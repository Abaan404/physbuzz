#pragma once

#include "shaders/quad.frag"
#include "shaders/quad.vert"

#include "../collision/collision.hpp"
#include "../dynamics/dynamics.hpp"
#include "objects.hpp"
#include <physbuzz/mesh.hpp>
#include <physbuzz/shaders.hpp>

struct QuadComponent {
    float width = 0.0f;
    float height = 0.0f;
};

struct QuadInfo {
    // physics info
    TransformableComponent transform;
    RigidBodyComponent body;

    // geometry
    QuadComponent quad;

    // naming
    IdentifiableComponent identifier = {
        .type = ObjectType::Box,
        .name = "Quad",
    };

    bool isCollidable = false;
    bool isRendered = false;
};

template <>
inline Physbuzz::ObjectID ObjectBuilder<QuadInfo>::build(Physbuzz::Object &object, QuadInfo &info) {
    // user-defined components
    object.setComponent(info.quad);
    object.setComponent(info.transform);
    object.setComponent(info.body);
    object.setComponent(info.identifier);

    // adhoc means to generate vertices for a quad without polluting the namespace
    auto generateVertices = [&]() {
        struct Mesh {
            std::vector<glm::vec3> vertices;
            std::vector<glm::uvec3> indices;
        } mesh;

        glm::vec3 min = info.transform.position - glm::vec3(info.quad.width / 2.0f, info.quad.height / 2.0f, 0.0f);
        glm::vec3 max = info.transform.position + glm::vec3(info.quad.width / 2.0f, info.quad.height / 2.0f, 0.0f);

        mesh.vertices = {
            glm::vec3(min.x, min.y, 0), // top-left
            glm::vec3(min.x, max.y, 0), // top-right
            glm::vec3(max.x, min.y, 0), // bottom-left
            glm::vec3(max.x, max.y, 0), // bottom-right
        };

        mesh.indices = {
            glm::vec3(0, 1, 2),
            glm::vec3(1, 2, 3),
        };

        return mesh;
    };

    // generate mesh
    if (info.isRendered) {
        Physbuzz::MeshComponent mesh = Physbuzz::MeshComponent();
        static Physbuzz::ShaderContext shader = Physbuzz::ShaderContext(quadVertex, quadFrag);
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

        if (info.isRendered) {
            vertices = object.getComponent<Physbuzz::MeshComponent>().getVertex();
        } else {
            vertices = generateVertices().vertices;
        }

        object.setComponent(AABBComponent::buildWithVertex(vertices));
    }

    return object.getId();
}
