#include "line.hpp"

#include <physbuzz/mesh.hpp>
#include <physbuzz/shaders.hpp>

#include "shaders/quad.frag"
#include "shaders/quad.vert"

template <>
Physbuzz::ObjectID ObjectBuilder<LineInfo>::build(Physbuzz::Object &object, LineInfo &info) {
    // user-defined components
    object.setComponent(info.line);
    object.setComponent(info.transform);
    object.setComponent(info.identifier);

    // generate mesh
    {
        glm::vec3 min = glm::vec3(-info.line.thickness / 2.0f, 0.0f, 0.0f);
        glm::vec3 max = glm::vec3(info.line.thickness / 2.0f, info.line.length, 0.0f);

        std::vector<glm::uvec3> indices = {
            {0, 1, 2},
            {0, 3, 2},
        };

        std::vector<glm::vec3> vertices = {
            {min.x, min.y, 0.0f}, // top-left
            {min.x, max.y, 0.0f}, // top-right
            {max.x, max.y, 0.0f}, // bottom-right
            {max.x, min.y, 0.0f}, // bottom-left
        };

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

    // build gl context
    if (info.isRenderable) {
        static Physbuzz::ShaderContext shader = Physbuzz::ShaderContext(quadVertex, quadFrag);
        static GLuint program = shader.load();

        Physbuzz::RenderComponent component = Physbuzz::RenderComponent();
        component.build();
        component.setProgram(program);

        object.setComponent(component);
    }

    // create a rebuild callback
    {
        RebuildableComponent component = {
            .rebuild = [](Physbuzz::Object &object) {
                if (object.hasComponent<Physbuzz::RenderComponent>()) {
                    object.getComponent<Physbuzz::RenderComponent>().destroy();
                }

                LineInfo info = {
                    .transform = object.getComponent<TransformableComponent>(),
                    .line = object.getComponent<LineComponent>(),
                    .identifier = object.getComponent<IdentifiableComponent>(),
                    .isRenderable = object.hasComponent<Physbuzz::RenderComponent>(),
                };

                object.eraseComponents();
                ObjectBuilder<LineInfo>::build(object, info);
            },
        };

        object.setComponent(component);
    }

    return object.getId();
}
