#include "line.hpp"

#include <physbuzz/shaders.hpp>

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
        Physbuzz::AABBComponent aabb = Physbuzz::AABBComponent(mesh, info.transform);
        generate2DTexCoords(aabb, mesh);
        generate2DNormals(mesh);

        mesh.build();
        object.setComponent(mesh);
    }

    // create a rebuild callback
    {
        RebuildableComponent rebuilder = {
            .rebuild = [](ObjectBuilder &builder, Physbuzz::Object &object) {
                if (object.hasComponent<Physbuzz::Mesh>()) {
                    object.getComponent<Physbuzz::Mesh>().destroy();
                }

                Line info = {
                    .transform = object.getComponent<Physbuzz::TransformableComponent>(),
                    .line = object.getComponent<LineComponent>(),
                    .identifier = object.getComponent<IdentifiableComponent>(),
                    .isRenderable = object.hasComponent<Physbuzz::Mesh>(),
                };

                object.eraseComponents();
                builder.create(object, info);
            },
        };

        object.setComponent(rebuilder);
    }

    return object.getId();
}
