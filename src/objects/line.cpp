#include "line.hpp"

#include <physbuzz/renderer.hpp>
#include <physbuzz/shaders.hpp>

#include "shaders/quad.frag"
#include "shaders/quad.vert"

template <>
Physbuzz::ObjectID ObjectBuilder::create(Physbuzz::Object &object, Line &info) {
    // user-defined components
    object.setComponent(info.line);
    object.setComponent(info.transform);
    object.setComponent(info.identifier);

    // generate mesh
    if (info.isRenderable) {
        glm::vec3 min = glm::vec3(-info.line.thickness / 2.0f, 0.0f, 0.0f);
        glm::vec3 max = glm::vec3(info.line.thickness / 2.0f, info.line.length, 0.0f);

        Physbuzz::Mesh mesh;
        mesh.vertices.resize(4);

        // calc positions
        std::vector<glm::vec3> positions = {
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

        for (std::size_t i = 0; i < mesh.vertices.size(); i++) {
            mesh.vertices[i].position = positions[i];
        }

        // calc vertices
        Physbuzz::BoundingComponent bounding = Physbuzz::BoundingComponent(mesh);
        generate2DTexCoords(bounding, mesh);
        generate2DNormals(mesh);

        // apply transformations
        applyTransformsToMesh(info.transform, mesh);

        // setup rendering
        Physbuzz::Texture texture = m_Textures.getTexture("quad");
        Physbuzz::ShaderPipeline shader = Physbuzz::ShaderPipeline(quadVertex, quadFrag);
        Physbuzz::RenderComponent render = Physbuzz::RenderComponent(mesh, shader, texture);
        render.build();

        object.setComponent(render);
    }

    // create a rebuild callback
    {
        RebuildableComponent rebuilder = {
            .rebuild = [](ObjectBuilder &builder, Physbuzz::Object &object) {
                if (object.hasComponent<Physbuzz::RenderComponent>()) {
                    object.getComponent<Physbuzz::RenderComponent>().destroy();
                }

                Line info = {
                    .transform = object.getComponent<Physbuzz::TransformableComponent>(),
                    .line = object.getComponent<LineComponent>(),
                    .identifier = object.getComponent<IdentifiableComponent>(),
                    .isRenderable = object.hasComponent<Physbuzz::RenderComponent>(),
                };

                object.eraseComponents();
                builder.create(object, info);
            },
        };

        object.setComponent(rebuilder);
    }

    return object.getId();
}
