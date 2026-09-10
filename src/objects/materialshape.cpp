#include "materialshape.hpp"

#include <physbuzz/graphics/transfer.hpp>
#include <physbuzz/physics/collision.hpp>
#include <physbuzz/render/defines.hpp>

template <>
Physbuzz::ObjectID ObjectBuilder::create(Physbuzz::Scene &scene, Physbuzz::ObjectID object, MaterialShape &info) {
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

    // naming
    IdentifiableComponent identifier = {
        .name = std::format("Material ({})", info.resources.material.getIdentifier()),
    };

    scene.setComponent(object, identifier, info.resources, render);

    return object;
}
