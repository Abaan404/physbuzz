#include "line.hpp"

#include <physbuzz/render/renderers/defines.hpp>
#include <physbuzz/shapes/square.hpp>

template <>
Physbuzz::ObjectID ObjectBuilder::create(Physbuzz::Scene &scene, Physbuzz::ObjectID object, Line &info) {
    // scale unit square for line
    info.transform.scale = {info.line.length, info.line.thickness, 0.0f};
    Physbuzz::Builtin::ModelSquare::build(scene.getSystem<Physbuzz::Transfer>());

    // setup rendering
    info.transform.update();
    Physbuzz::RenderComponent render = {
        .transform = info.transform,
        .model = Physbuzz::Model::Info{
            .meshes = {
                {
                    .materialIdx = 0,
                    .resource = Physbuzz::Builtin::ModelSquare::Resource,
                },
            },
            .materials = {},
        },
    };

    // create a rebuild callback
    RebuildableComponent rebuilder = {
        .rebuild = [](Physbuzz::Scene &scene, Physbuzz::ObjectID object) {
            if (!scene.containsComponent<LineComponent, IdentifiableComponent, ResourceComponent, Physbuzz::RenderComponent>(object)) {
                Physbuzz::Logger::ERROR("[RebuildableComponent] Cannot rebuild object with id '{}' with missing core components.", object);
                return;
            }

            const auto [line, identifier, resources, render] = scene.getComponent<LineComponent, IdentifiableComponent, ResourceComponent, Physbuzz::RenderComponent>(object);

            Line info = {
                .line = line,
                .transform = render.transform,
                .identifier = identifier,
                .resources = resources,
            };

            ObjectBuilder::create(scene, object, info);
        },
    };

    scene.setComponent(object, info.line, info.identifier, info.resources, render, rebuilder);

    return object;
}
