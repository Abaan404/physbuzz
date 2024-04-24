#include "../objects.hpp"

#include "../../dynamics/dynamics.hpp"
#include "../../renderer/mesh.hpp"
#include "../../renderer/shaders.hpp"

#include "shaders/box.vert"
#include "shaders/box.frag"

Object &create_box(Scene &scene, glm::vec3 position, float width, float height) {
    Object &box = scene.create_object();

    // box props
    {
        AABBComponent aabb = {
            .min = glm::vec3(position[0] - (width / 2.0f), position[1] - (height / 2.0f), 0.0f),
            .max = glm::vec3(position[0] + (width / 2.0f), position[1] + (height / 2.0f), 0.0f),
        };

        box.add_component(aabb);
    }

    // transform
    {
        TransformableComponent transform = {
            .position = position,
        };
        box.add_component(transform);
    }

    // physics
    {
        RigidBodyComponent physics = {
            .velocity = glm::vec3(0.0f, 0.0f, 0.0f),
            .acceleration = glm::vec3(0.0f, 0.0f, 0.0f),
        };
        box.add_component(physics);
    }

    // mesh
    {
        MeshComponent mesh = MeshComponent();
        AABBComponent aabb = box.get_component<AABBComponent>();
        static ShaderContext shader = ShaderContext(box_vertex, box_frag);
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

        mesh.normalized = false;
        mesh.set_program(program);
        mesh.set_vertex(vertices);
        mesh.set_index(indices);

        // todo uniforms
        // glUseProgram(mesh.get_program());
        // glUniform4f(mesh.glu_color, 0.5f, 0.0f, 0.0f, 0.0f);

        box.add_component(mesh);
    }

    return box;
}
