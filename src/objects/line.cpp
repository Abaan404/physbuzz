#include "line.hpp"

#include <physbuzz/render/shaders.hpp>

template <>
Physbuzz::ObjectID ObjectBuilder::create(Physbuzz::ObjectID object, Line &info) {
    // user-defined components
    scene->setComponent(object, info.line, info.identifier);

    // generate mesh
    if (info.isRenderable) {
        glm::vec3 min = glm::vec3(-info.line.thickness / 2.0f, 0.0f, 0.0f);
        glm::vec3 max = glm::vec3(info.line.thickness / 2.0f, info.line.length, 0.0f);

        Physbuzz::MeshComponent mesh = Physbuzz::MeshComponent(info.model, info.material);
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
        generate2DTexCoords(mesh);
        generate2DNormals(mesh);

        mesh.build();
        scene->setComponent(object, mesh);
    }

    // create a rebuild callback
    {
        RebuildableComponent rebuilder = {
            .rebuild = [](ObjectBuilder &builder, Physbuzz::ObjectID object) {
                Line info = {
                    .line = builder.scene->getComponent<LineComponent>(object),
                    .identifier = builder.scene->getComponent<IdentifiableComponent>(object),
                    .resources = builder.scene->getComponent<ResourceIdentifierComponent>(object),
                    .isRenderable = false,
                };

                if (builder.scene->containsComponent<Physbuzz::MeshComponent>(object)) {
                    info.isRenderable = true;
                    info.model = builder.scene->getComponent<Physbuzz::MeshComponent>(object).model;
                    info.material = builder.scene->getComponent<Physbuzz::MeshComponent>(object).material;
                }

                builder.create(object, info);
            },
        };

        scene->setComponent(object, rebuilder);
    }

    return object;
}
