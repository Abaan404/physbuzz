#include "lightcube.hpp"

#include <physbuzz/physics/collision.hpp>
#include <physbuzz/render/lighting.hpp>
#include <physbuzz/render/model.hpp>

template <>
Physbuzz::ObjectID ObjectBuilder::create(Physbuzz::ObjectID object, LightCube &info) {
    // create a cube and add its component to this object
    create(object, info.cube);

    // add a point light to the center of the cube
    Physbuzz::PointLightComponent pointLight = {
        .position = {
            info.cube.transform.position.x,
            info.cube.transform.position.y,
            info.cube.transform.position.z,
        },

        .ambient = info.pointLight.ambient,
        .diffuse = info.pointLight.diffuse,
        .specular = info.pointLight.specular,

        .constant = info.pointLight.constant,
        .linear = info.pointLight.linear,
        .quadratic = info.pointLight.quadratic,
    };

    // create a rebuild callback
    RebuildableComponent rebuilder = {
        .rebuild = [](ObjectBuilder &builder, Physbuzz::ObjectID object) {
            if (!builder.scene->containsComponent<CubeComponent, IdentifiableComponent, ResourceComponent, Physbuzz::PointLightComponent, Physbuzz::RenderComponent>(object)) {
                Physbuzz::Logger::ERROR("[RebuildableComponent] Cannot rebuild object with id '{}' with missing core components.", object);
                return;
            }

            const auto &[cube, identifier, resources, pointlight, render] = builder.scene->getComponent<CubeComponent, IdentifiableComponent, ResourceComponent, Physbuzz::PointLightComponent, Physbuzz::RenderComponent>(object);

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

    scene->setComponent(object, pointLight, rebuilder);

    return object;
}
