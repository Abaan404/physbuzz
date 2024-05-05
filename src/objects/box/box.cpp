#include "../objects.hpp"

#include "../../dynamics/dynamics.hpp"
#include <physbuzz/mesh.hpp>
#include <physbuzz/shaders.hpp>

#include "shaders/box.frag"
#include "shaders/box.vert"

void buildBox(Physbuzz::Object &box, glm::vec3 position, float width, float height) {
    // name
    {
        IdentifiableComponent identifiable = {
            .type = ObjectType::Box,
            .name = "Box",
        };

        box.setComponent(identifiable);
    }

    // box props
    {
        AABBComponent aabb = {
            .min = glm::vec3(position[0] - (width / 2.0f), position[1] - (height / 2.0f), 0.0f),
            .max = glm::vec3(position[0] + (width / 2.0f), position[1] + (height / 2.0f), 0.0f),
        };

        box.setComponent(aabb);
    }

    // transform
    {
        TransformableComponent transform = {
            .position = position,
        };
        box.setComponent(transform);
    }

    // physics
    {
        RigidBodyComponent physics = {
            .mass = 1.0f,
            .velocity = glm::vec3(0.0f, 0.0f, 0.0f),
            .acceleration = glm::vec3(0.0f, 0.0f, 0.0f),
        };
        box.setComponent(physics);
    }

    // mesh
    {
        Physbuzz::MeshComponent mesh = Physbuzz::MeshComponent();
        AABBComponent aabb = box.getComponent<AABBComponent>();
        static Physbuzz::ShaderContext shader = Physbuzz::ShaderContext(boxVertex, boxFrag);
        static GLuint program = shader.load();

        std::vector<glm::vec3> vertices = {
            glm::vec3(aabb.min.x, aabb.min.y, 0), // top-left
            glm::vec3(aabb.min.x, aabb.max.y, 0), // top-right
            glm::vec3(aabb.max.x, aabb.min.y, 0), // bottom-left
            glm::vec3(aabb.max.x, aabb.max.y, 0), // bottom-right
        };

        std::vector<glm::uvec3> indices = {
            glm::vec3(0, 1, 2),
            glm::vec3(1, 2, 3),
        };

        mesh.build();
        mesh.setProgram(program);
        mesh.setVertex(vertices);
        mesh.setIndex(indices);

        // todo uniforms
        // glUseProgram(mesh.get_program());
        // glUniform4f(mesh.glu_color, 0.5f, 0.0f, 0.0f, 0.0f);

        box.setComponent(mesh);
    }
}
