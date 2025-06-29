#include "line.hpp"

#include <physbuzz/render/model.hpp>
#include <physbuzz/render/renderer.hpp>

template <>
Physbuzz::ObjectID ObjectBuilder::create(Physbuzz::ObjectID object, Line &info) {
    // generate mesh
    glm::vec3 min = glm::vec3(-info.line.thickness / 2.0f, 0.0f, 0.0f);
    glm::vec3 max = glm::vec3(info.line.thickness / 2.0f, info.line.length, 0.0f);

    Physbuzz::Mesh::Info mesh = {
        .vertices = {
            {{min.x, min.y, 0.0f}, {}, {}}, // top-left
            {{min.x, max.y, 0.0f}, {}, {}}, // top-right
            {{max.x, max.y, 0.0f}, {}, {}}, // bottom-right
            {{max.x, min.y, 0.0f}, {}, {}}, // bottom-left
        },
        .indices = {0, 1, 2, 0, 3, 2},
    };

    generateTexCoords(mesh);
    generateNormals(mesh);

    // create model
    std::string modelName = std::format("line_{}", object);
    Physbuzz::ResourceRegistry<Physbuzz::Model>::insert(
        modelName,
        {{
            .meshes = {{mesh, {}}},
            .textures = info.resources.textures,
        }});

    // setup rendering
    info.transform.update();
    Physbuzz::RenderComponent render = {
        .transform = info.transform,
        .model = modelName,
        .pipeline = info.resources.pipeline,
    };

    info.transform.update();

    // create a rebuild callback
    RebuildableComponent rebuilder = {
        .rebuild = [](ObjectBuilder &builder, Physbuzz::ObjectID object) {
            if (!builder.scene->containsComponent<LineComponent, IdentifiableComponent, ResourceComponent, Physbuzz::RenderComponent>(object)) {
                Physbuzz::Logger::ERROR("[RebuildableComponent] Cannot rebuild object with id '{}' with missing core components.", object);
                return;
            }

            const auto [line, identifier, resources, render] = builder.scene->getComponent<LineComponent, IdentifiableComponent, ResourceComponent, Physbuzz::RenderComponent>(object);

            Line info = {
                .line = line,
                .transform = render.transform,
                .identifier = identifier,
                .resources = resources,
            };

            builder.create(object, info);
        },
    };

    scene->setComponent(object, info.line, info.identifier, info.resources, render, rebuilder);

    return object;
}
