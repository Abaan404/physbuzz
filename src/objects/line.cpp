#include "line.hpp"

#include <physbuzz/render/shaders.hpp>

template <>
Physbuzz::ObjectID ObjectBuilder::create(Physbuzz::ObjectID object, Line &info) {
    // user-defined components
    scene->setComponent(object, info.line, info.transform, info.identifier);

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
        scene->setComponent(object, mesh);
    }

    // create a rebuild callback
    {
        RebuildableComponent rebuilder = {
            .rebuild = [](ObjectBuilder &builder, Physbuzz::ObjectID object) {
                Line info = {
                    .transform = builder.scene->getComponent<Physbuzz::TransformableComponent>(object),
                    .line = builder.scene->getComponent<LineComponent>(object),
                    .identifier = builder.scene->getComponent<IdentifiableComponent>(object),
                    .isRenderable = builder.scene->containsComponent<Physbuzz::Mesh>(object),
                };

                builder.create(object, info);
            },
        };

        scene->setComponent(object, rebuilder);
    }

    return object;
}
