#include "shape.hpp"

#include <physbuzz/graphics/transfer.hpp>
#include <physbuzz/physics/collision.hpp>
#include <physbuzz/physics/dynamics.hpp>
#include <physbuzz/render/defines.hpp>
#include <physbuzz/render/shadow.hpp>

template <>
Physbuzz::ObjectID ObjectBuilder::create(Physbuzz::Scene &scene, Physbuzz::ObjectID object, Shape &info) {
    // setup rendering
    Physbuzz::RenderComponent render = {
        .renderable = {
            .model = {{
                .mesh = info.resources.mesh,
                .materials = {info.resources.material},
                .submeshMaterialIndices = {0},
            }},
            .transform = info.transform,
        },
    };

    scene.setComponent(object, info.identifier, info.resources, render);

    if (info.hasShadow) {
        Physbuzz::ShadowComponent shadow;
        scene.setComponent(object, shadow);
    }

    if (info.hasPhysics) {
        Physbuzz::RigidBodyComponent body = {info.body};
        body.applyTransform(info.transform);

        scene.setComponent(object, body);
    }

    return object;
}
