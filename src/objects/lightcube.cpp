#include "lightcube.hpp"

#include <physbuzz/physics/collision.hpp>
#include <physbuzz/render/lighting.hpp>

template <>
Physbuzz::ObjectID ObjectBuilder::create(Physbuzz::ObjectID object, LightCube &info) {
    // create a cube and add its component to this object
    create(object, info.cube);

    // add a point light to the center of the cube
    Physbuzz::PointLightComponent pointLight = {
        .position = {
            info.cube.transform.position.x + info.cube.cube.width / 2.0f,
            info.cube.transform.position.y + info.cube.cube.height / 2.0f,
            info.cube.transform.position.z + info.cube.cube.length / 2.0f,
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
            if (!builder.scene->containsComponent<CubeComponent, Physbuzz::TransformComponent, IdentifiableComponent, Physbuzz::ModelComponent, ResourceComponent>(object)) {
                Physbuzz::Logger::ERROR("[RebuildableComponent] Cannot rebuild object with id '{}' with missing core components.", object);
                return;
            }

            // NOTE: rebuilder framework may need a rewrite
            LightCube info = {
                .cube = {
                    // .body = object.getComponent<Physbuzz::RigidBodyComponent>(),
                    .cube = builder.scene->getComponent<CubeComponent>(object),
                    .transform = builder.scene->getComponent<Physbuzz::TransformComponent>(object),
                    .identifier = builder.scene->getComponent<IdentifiableComponent>(object),
                    .resources = builder.scene->getComponent<ResourceComponent>(object),
                    .hasPhysics = false,
                },
                .pointLight = builder.scene->getComponent<Physbuzz::PointLightComponent>(object),
            };

            builder.create(object, info);
        },
    };

    scene->setComponent(object, pointLight, rebuilder);

    return object;
}
