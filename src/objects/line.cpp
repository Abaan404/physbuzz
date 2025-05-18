#include "line.hpp"

#include <physbuzz/render/model.hpp>
#include <physbuzz/resources/manager.hpp>

template <>
Physbuzz::ObjectID ObjectBuilder::create(Physbuzz::ObjectID object, Line &info) {
    // generate mesh
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
    mesh.indices = {0, 1, 2, 0, 3, 2};

    for (std::size_t i = 0; i < mesh.vertices.size(); i++) {
        mesh.vertices[i].position = positions[i];
    }

    // calc vertices
    generate2DTexCoords(mesh);
    generate2DNormals(mesh);

    // add textures
    mesh.textures = info.resources.textures;

    // create model
    std::string model = std::format("quad_{}", object);
    Physbuzz::ResourceRegistry<Physbuzz::ModelResource>::insert(model, {{mesh}});

    // setup rendering
    Physbuzz::ModelComponent render = {
        .model = model,
    };

    info.transform.update();

    // create a rebuild callback
    RebuildableComponent rebuilder = {
        .rebuild = [](ObjectBuilder &builder, Physbuzz::ObjectID object) {
            if (!builder.scene->containsComponent<LineComponent, Physbuzz::TransformComponent, IdentifiableComponent, Physbuzz::ModelComponent>(object)) {
                Physbuzz::Logger::ERROR("[RebuildableComponent] Cannot rebuild object with id '{}' with missing core components.", object);
                return;
            }

            Line info = {
                .line = builder.scene->getComponent<LineComponent>(object),
                .transform = builder.scene->getComponent<Physbuzz::TransformComponent>(object),
                .identifier = builder.scene->getComponent<IdentifiableComponent>(object),
                .resources = builder.scene->getComponent<ResourceComponent>(object),
            };

            builder.create(object, info);
        },
    };

    scene->setComponent(object, info.line, info.identifier, info.resources, info.transform, render, rebuilder);

    return object;
}
