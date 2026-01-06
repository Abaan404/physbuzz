#include "lightcuboid.hpp"

#include <physbuzz/render/renderers/defines.hpp>
#include <physbuzz/shapes/cube.hpp>

template <>
Physbuzz::ObjectID ObjectBuilder::create(Physbuzz::Scene &scene, Physbuzz::ObjectID object, LightCuboid &info) {
    // scale unit cube for cuboid
    info.transform.scale = {info.cuboid.width, info.cuboid.breadth, info.cuboid.height};
    Physbuzz::Builtin::ModelCube::build(scene.getSystem<Physbuzz::Transfer>());

    // setup rendering
    info.transform.update();
    Physbuzz::RenderComponent render = {
        .transform = info.transform,
        .model = Physbuzz::Model::Info{
            .meshes = {
                {
                    .materialIdx = 0,
                    .resource = Physbuzz::Builtin::ModelCube::Resource,
                },
            },
            .materials = {},
        },
    };

    // point light matches this object's transform
    info.pointLight.position = {
        info.transform.position.x,
        info.transform.position.y,
        info.transform.position.z,
    };

    // create a rebuild callback
    RebuildableComponent rebuilder = {
        .rebuild = [](Physbuzz::Scene &scene, Physbuzz::ObjectID object) {
            if (!scene.containsComponent<CuboidComponent, IdentifiableComponent, ResourceComponent, Physbuzz::RenderComponent>(object)) {
                Physbuzz::Logger::ERROR("[RebuildableComponent] Cannot rebuild object with id '{}' with missing core components.", object);
                return;
            }

            const auto [cuboid, identifier, resources, render, pointLight] = scene.getComponent<CuboidComponent, IdentifiableComponent, ResourceComponent, Physbuzz::RenderComponent, Physbuzz::PointLightComponent>(object);

            LightCuboid info = {
                .cuboid = cuboid,
                .transform = render.transform,
                .identifier = identifier,
                .resources = resources,
                .pointLight = pointLight,
            };

            ObjectBuilder::create(scene, object, info);
        },
    };

    scene.setComponent(object, info.cuboid, info.identifier, info.resources, info.pointLight, render, rebuilder);

    return object;
}
