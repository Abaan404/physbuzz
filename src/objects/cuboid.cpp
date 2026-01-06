#include "cuboid.hpp"

#include <physbuzz/physics/collision.hpp>
#include <physbuzz/physics/dynamics.hpp>
#include <physbuzz/render/renderers/defines.hpp>
#include <physbuzz/render/shadow.hpp>
#include <physbuzz/shapes/cube.hpp>

template <>
Physbuzz::ObjectID ObjectBuilder::create(Physbuzz::Scene &scene, Physbuzz::ObjectID object, Cuboid &info) {
    // scale unit cube for cuboid
    info.transform.scale = {info.cuboid.width, info.cuboid.breadth, info.cuboid.height};
    Physbuzz::Builtin::ModelCube::build(scene.getSystem<Physbuzz::Transfer>());

    // setup rendering
    info.transform.update();
    Physbuzz::ShadowComponent shadow;
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

    // create a rebuild callback
    RebuildableComponent rebuilder = {
        .rebuild = [](Physbuzz::Scene &scene, Physbuzz::ObjectID object) {
            if (!scene.containsComponent<CuboidComponent, IdentifiableComponent, ResourceComponent, Physbuzz::RenderComponent>(object)) {
                Physbuzz::Logger::ERROR("[RebuildableComponent] Cannot rebuild object with id '{}' with missing core components.", object);
                return;
            }

            const auto [cuboid, identifier, resources, render] = scene.getComponent<CuboidComponent, IdentifiableComponent, ResourceComponent, Physbuzz::RenderComponent>(object);

            Cuboid info = {
                // .body = object.getComponent<Physbuzz::RigidBodyComponent>(),
                .cuboid = cuboid,
                .transform = render.transform,
                .identifier = identifier,
                .resources = resources,
                .hasPhysics = scene.containsComponent<Physbuzz::RigidBodyComponent>(object),
            };

            if (info.hasPhysics) {
                // info.body = scene.getComponent<Physbuzz::RigidBodyComponent>(object);
            }

            ObjectBuilder::create(scene, object, info);
        },
    };

    scene.setComponent(object, info.cuboid, info.identifier, info.resources, render, shadow, rebuilder);

    // if (info.hasPhysics) {
    //     // generate bounding box
    //     Physbuzz::AABBComponent aabb = Physbuzz::AABBComponent(render);
    //     scene.setComponent(object, aabb);
    // }

    return object;
}
