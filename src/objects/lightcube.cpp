#include "lightcube.hpp"

#include <physbuzz/physics/collision.hpp>
#include <physbuzz/render/lighting.hpp>

template <>
Physbuzz::ObjectID ObjectBuilder::create(Physbuzz::ObjectID object, LightCube &info) {
    // create a cube and add its component to this object
    create(object, info.cube);

    // add a point light to the center of the cube
    if (info.cube.isRenderable) {
        Physbuzz::PointLightComponent pointLight = {
            .position = {
                info.cube.model.position.x + info.cube.cube.width / 2.0f,
                info.cube.model.position.y + info.cube.cube.height / 2.0f,
                info.cube.model.position.z + info.cube.cube.length / 2.0f,
            },

            .ambient = info.pointLight.ambient,
            .diffuse = info.pointLight.diffuse,
            .specular = info.pointLight.specular,

            .constant = info.pointLight.constant,
            .linear = info.pointLight.linear,
            .quadratic = info.pointLight.quadratic,
        };

        scene->setComponent(object, pointLight);
    }

    // create a rebuild callback
    {
        RebuildableComponent rebuilder = {
            .rebuild = [](ObjectBuilder &builder, Physbuzz::ObjectID object) {
                // NOTE: rebuilder framework may need a rewrite
                LightCube info = {
                    .cube = {
                        // .body = object.getComponent<Physbuzz::RigidBodyComponent>(),
                        .cube = builder.scene->getComponent<CubeComponent>(object),
                        .identifier = builder.scene->getComponent<IdentifiableComponent>(object),
                        .resources = builder.scene->getComponent<ResourceIdentifierComponent>(object),
                        .isCollidable = builder.scene->containsComponent<Physbuzz::AABBComponent>(object),
                        .isRenderable = false,
                    },
                    .pointLight = builder.scene->getComponent<Physbuzz::PointLightComponent>(object),
                };

                if (builder.scene->containsComponent<Physbuzz::MeshComponent>(object)) {
                    info.cube.isRenderable = true;
                    info.cube.model = builder.scene->getComponent<Physbuzz::MeshComponent>(object).model;
                    info.cube.material = builder.scene->getComponent<Physbuzz::MeshComponent>(object).material;
                }

                builder.create(object, info);
            },
        };

        scene->setComponent(object, rebuilder);
    }

    return object;
}
