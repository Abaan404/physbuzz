#include "lightcube.hpp"

#include <physbuzz/render/shadow.hpp>

template <>
Physbuzz::ObjectID ObjectBuilder::create(Physbuzz::ObjectID object, LightCube &info) {
    // create a cube and add its component to this object
    create(object, info.cube);

    // dont cast shadows on this cube
    scene->eraseComponent<Physbuzz::ShadowComponent>(object);

    // add a point light to the center of the cube
    info.pointLight.position = {
        info.cube.transform.position.x,
        info.cube.transform.position.y,
        info.cube.transform.position.z,
    };

    // create a rebuild callback
    RebuildableComponent rebuilder = {
        .rebuild = [](ObjectBuilder &builder, Physbuzz::ObjectID object) {
            if (!builder.scene->containsComponent<CubeComponent, IdentifiableComponent, ResourceComponent, Physbuzz::PointLightComponent, Physbuzz::RenderComponent>(object)) {
                Physbuzz::Logger::ERROR("[RebuildableComponent] Cannot rebuild object with id '{}' with missing core components.", object);
                return;
            }

            const auto [cube, identifier, resources, pointlight, render] = builder.scene->getComponent<CubeComponent, IdentifiableComponent, ResourceComponent, Physbuzz::PointLightComponent, Physbuzz::RenderComponent>(object);

            // NOTE: rebuilder framework may need a rewrite
            LightCube info = {
                .cube = {
                    // .body = object.getComponent<Physbuzz::RigidBodyComponent>(),
                    .cube = cube,
                    .transform = render.transform,
                    .identifier = identifier,
                    .resources = resources,
                    .hasPhysics = false,
                },
                .pointLight = pointlight,
            };

            builder.create(object, info);
        },
    };

    scene->setComponent(object, info.pointLight, rebuilder);

    return object;
}
