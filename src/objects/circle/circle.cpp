#include "../objects.hpp"

#include "../../collision/collision.hpp"
#include "../../dynamics/dynamics.hpp"
#include <glm/ext/scalar_constants.hpp>
#include <physbuzz/mesh.hpp>
#include <physbuzz/shaders.hpp>

#include "shaders/circle.frag"
#include "shaders/circle.vert"

#define MAX_VERTICES 50

void buildCircle(Physbuzz::Object &circle, glm::vec3 position, float radius) {
    // name
    {
        IdentifiableComponent component = {
            .type = ObjectType::Circle,
            .name = "Circle",
        };

        circle.setComponent(component);
    }

    // circle props
    {
        CircleComponent component = {
            .radius = radius,
        };

        circle.setComponent(component);
    }

    // transform
    {
        TransformableComponent component = {
            .position = position,
        };

        circle.setComponent(component);
    }

    // physics
    {
        RigidBodyComponent component = {
            .mass = 1.0f,
            .velocity = glm::vec3(0.01f, 0.01f, 0.0f),
            .acceleration = glm::vec3(0.0f, 0.0f, 0.0f),
        };

        circle.setComponent(component);
    }

    // gravity
    {
        GravityComponent component = {};
        circle.setComponent(component);
    }

    // drag
    {
        DragComponent component = {};
        circle.setComponent(component);
    }

    // bounding box
    {
        AABBComponent component = {
            .min = position - radius,
            .max = position + radius,
        };

        circle.setComponent(component);
    }

    // mesh
    {
        Physbuzz::MeshComponent mesh = Physbuzz::MeshComponent();
        static Physbuzz::ShaderContext shader = Physbuzz::ShaderContext(circleVertex, circleFrag);

        static GLuint program = shader.load();

        std::vector<glm::vec3> vertices;
        std::vector<glm::uvec3> indices;

        static float angleIncrement = (2.0f * glm::pi<float>()) / MAX_VERTICES;

        // good ol trig
        TransformableComponent transform = circle.getComponent<TransformableComponent>();
        for (int i = 0; i < MAX_VERTICES; i++) {
            float angle = i * angleIncrement;
            float x = transform.position.x + radius * glm::cos(angle);
            float y = transform.position.y + radius * glm::sin(angle);
            vertices.push_back(glm::vec3(x, y, 0));
        }

        mesh.build();
        mesh.setProgram(program);
        mesh.setVertex(vertices);
        mesh.setIndex(indices);

        for (int i = 1; i < MAX_VERTICES - 1; i++) {
            indices.push_back(glm::uvec3(0, i, i + 1));
        }
        indices.push_back(glm::uvec3(0, MAX_VERTICES - 1, 1));

        mesh.setVertex(vertices);
        mesh.setIndex(indices);

        // todo uniforms
        // glUseProgram(program);
        // glUniform4f(gl_color, 0.5f, 0.0f, 0.0f, 0.0f);

        circle.setComponent(mesh);
    }
}