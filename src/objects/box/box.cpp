#include "../objects.hpp"

#include "../../collision/collision.hpp"
#include "../../dynamics/dynamics.hpp"
#include <physbuzz/mesh.hpp>
#include <physbuzz/shaders.hpp>

#include "shaders/box.frag"
#include "shaders/box.vert"

void buildBox(Physbuzz::Object &box, glm::vec3 position, float width, float height) {
    // name
    {
        IdentifiableComponent component = {
            .type = ObjectType::Box,
            .name = "Box",
        };

        box.setComponent(component);
    }

    // box props
    {
        QuadComponent component = {
            .width = width,
            .height = height,
        };

        box.setComponent(component);
    }

    // transform
    {
        TransformableComponent component = {
            .position = position,
        };

        box.setComponent(component);
    }

    // physics
    {
        RigidBodyComponent component = {
            .mass = 1.0f,
            .velocity = glm::vec3(0.0f, 0.0f, 0.0f),
            .acceleration = glm::vec3(0.0f, 0.0f, 0.0f),
        };

        box.setComponent(component);
    }

    // bounding box
    {
        AABBComponent component = {
            .min = glm::vec3(position[0] - (width / 2.0f), position[1] - (height / 2.0f), 0.0f),
            .max = glm::vec3(position[0] + (width / 2.0f), position[1] + (height / 2.0f), 0.0f),
        };

        box.setComponent(component);
    }

    // mesh
    {
        Physbuzz::MeshComponent mesh = Physbuzz::MeshComponent();
        static Physbuzz::ShaderContext shader = Physbuzz::ShaderContext(boxVertex, boxFrag);
        static GLuint program = shader.load();

        TransformableComponent &transform = box.getComponent<TransformableComponent>();
        glm::vec3 min = position - glm::vec3(width / 2.0f, height / 2.0f, 0.0f);
        glm::vec3 max = position + glm::vec3(width / 2.0f, height / 2.0f, 0.0f);
        std::vector<glm::vec3> vertices = {
            glm::vec3(min.x, min.y, 0), // top-left
            glm::vec3(min.x, max.y, 0), // top-right
            glm::vec3(max.x, min.y, 0), // bottom-left
            glm::vec3(max.x, max.y, 0), // bottom-right
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
