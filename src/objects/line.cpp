#include "line.hpp"

#include <physbuzz/renderer.hpp>
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
    if (info.isRenderable) {
        glm::vec3 min = glm::vec3(-info.line.thickness / 2.0f, 0.0f, 0.0f);
        glm::vec3 max = glm::vec3(info.line.thickness / 2.0f, info.line.length, 0.0f);

        Physbuzz::Mesh mesh;

        // calc positions
        mesh.positions = {
            {min.x, min.y, 0.0f}, // top-left
            {min.x, max.y, 0.0f}, // top-right
            {max.x, max.y, 0.0f}, // bottom-right
            {max.x, min.y, 0.0f}, // bottom-left
        };

        // calc indices
        mesh.indices = {
            {0, 1, 2},
            {0, 3, 2},
        };

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
        Physbuzz::ShaderPipeline shader = Physbuzz::ShaderPipeline(quadVertex, quadFrag);
        Physbuzz::RenderComponent render = Physbuzz::RenderComponent(mesh, shader);
        render.build();

        object.setComponent(render);
    }

    // create a rebuild callback
    {
        RebuildableComponent rebuilder = {
            .rebuild = [](Physbuzz::Object &object) {
                if (object.hasComponent<Physbuzz::RenderComponent>()) {
                    object.getComponent<Physbuzz::RenderComponent>().destroy();
                }

                LineInfo info = {
                    .transform = object.getComponent<Physbuzz::TransformableComponent>(),
                    .line = object.getComponent<LineComponent>(),
                    .identifier = object.getComponent<IdentifiableComponent>(),
                    .isRenderable = object.hasComponent<Physbuzz::RenderComponent>(),
                };

                object.eraseComponents();
                ObjectBuilder<LineInfo>::build(object, info);
            },
        };

        object.setComponent(rebuilder);
    }

    return object.getId();
}
