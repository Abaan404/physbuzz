#include "../objects.hpp"

#include "../../dynamics/dynamics.hpp"
#include "../../renderer/mesh.hpp"
#include "../../renderer/shaders.hpp"

#include "shaders/circle.frag"
#include "shaders/circle.vert"

#include <glm/ext/scalar_constants.hpp>

#define MAX_VERTICES 50

Object &create_circle(Scene &scene, glm::vec3 position, float radius) {
    Object &circle = scene.create_object();

    // circle props
    {
        RadiusComponent circ = {
            .radius = radius,
        };

        circle.add_component(circ);
    }

    // transform
    {
        TransformableComponent transform = {
            .position = position,
        };
        circle.add_component(transform);
    }

    // physics
    {
        RigidBodyComponent physics = {
            .velocity = glm::vec3(0.01f, 0.01f, 0.0f),
            .acceleration = glm::vec3(0.0f, 0.0f, 0.0f),
        };
        circle.add_component(physics);
    }

    // mesh
    {
        MeshComponent mesh = MeshComponent();
        static ShaderContext shader = ShaderContext(circle_vertex, circle_frag);

        static GLuint program = shader.load();

        std::vector<glm::vec3> vertex_buffer;
        std::vector<glm::uvec3> index_buffer;

        static float angle_increment = (2.0f * glm::pi<float>()) / MAX_VERTICES;

        // good ol trig
        TransformableComponent transform = circle.get_component<TransformableComponent>();
        for (int i = 0; i < MAX_VERTICES; i++) {
            float angle = i * angle_increment;
            float x = transform.position.x + radius * glm::cos(angle);
            float y = transform.position.y + radius * glm::sin(angle);
            vertex_buffer.push_back(glm::vec3(x, y, 0));
        }

        mesh.normalized = false;
        mesh.set_program(program);
        mesh.set_vertex(vertex_buffer);
        mesh.set_index(index_buffer);


        for (int i = 1; i < MAX_VERTICES - 1; i++) {
            index_buffer.push_back(glm::uvec3(0, i, i + 1));
        }
        index_buffer.push_back(glm::uvec3(0, MAX_VERTICES - 1, 1));

        mesh.set_vertex(vertex_buffer);
        mesh.set_index(index_buffer);

        // todo uniforms
        // glUseProgram(program);
        // glUniform4f(gl_color, 0.5f, 0.0f, 0.0f, 0.0f);

        circle.add_component(mesh);
    }

    return circle;
}
