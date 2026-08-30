#include "lightpoint.hpp"

#include <physbuzz/graphics/transfer.hpp>
#include <physbuzz/render/defines.hpp>
#include <physbuzz/shapes/sphere.hpp>

template <>
Physbuzz::ObjectID ObjectBuilder::create(Physbuzz::Scene &scene, Physbuzz::ObjectID object, LightPoint &info) {
    // scale unit sphere
    info.transform.setScale({info.sphere.radius, info.sphere.radius, info.sphere.radius});

    // setup rendering
    Physbuzz::RenderComponent render = {
        .transform = info.transform,
        .model = {{
            .mesh = Physbuzz::Builtin::ModelSphere::Resource,
            .materials = {info.resources.material},
            .submeshMaterialIndices = {0},
        }},
    };

    // point light matches this object's transform
    info.pointLight.setPosition({
        info.transform.getInfo().position.x,
        info.transform.getInfo().position.y,
        info.transform.getInfo().position.z,
    });

    scene.setComponent(object, info.sphere, info.identifier, info.resources, info.pointLight, render);

    return object;
}
