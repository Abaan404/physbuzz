#include "cuboid.hpp"

#include <physbuzz/graphics/transfer.hpp>
#include <physbuzz/physics/collision.hpp>
#include <physbuzz/physics/dynamics.hpp>
#include <physbuzz/render/defines.hpp>
#include <physbuzz/render/shadow.hpp>
#include <physbuzz/shapes/cube.hpp>

template <>
Physbuzz::ObjectID ObjectBuilder::create(Physbuzz::Scene &scene, Physbuzz::ObjectID object, Cuboid &info) {
    // scale unit cube for cuboid
    info.transform.setScale({info.cuboid.width, info.cuboid.breadth, info.cuboid.height});

    // setup rendering
    Physbuzz::ShadowComponent shadow;
    Physbuzz::RenderComponent render = {
        .transform = info.transform,
        .model = {{
            .mesh = Physbuzz::Builtin::ModelCube::Resource,
            .materials = {info.resources.material},
            .submeshMaterialIndices = {0},
        }},
    };

    scene.setComponent(object, info.cuboid, info.identifier, info.resources, render, shadow);

    // if (info.hasPhysics) {
    //     // generate bounding box
    //     Physbuzz::AABBComponent aabb = Physbuzz::AABBComponent(render);
    //     scene.setComponent(object, aabb);
    // }

    return object;
}
