#include "lightpoint.hpp"

#include <physbuzz/graphics/transfer.hpp>
#include <physbuzz/render/defines.hpp>

template <>
Physbuzz::ObjectID ObjectBuilder::create(Physbuzz::Scene &scene, Physbuzz::ObjectID object, LightPoint &info) {
    // setup rendering
    Physbuzz::RenderComponent render = {
        .model = {{
            .mesh = info.resources.mesh,
            .materials = {info.resources.material},
            .submeshMaterialIndices = {0},
        }},
    };

    Physbuzz::TransformComponent transform = {
        .transform = info.transform,
    };

    // point light matches this object's transform
    info.pointLight.setPosition(info.transform.getInfo().position);

    scene.setComponent(object, info.identifier, info.resources, info.pointLight, render, transform);

    return object;
}
