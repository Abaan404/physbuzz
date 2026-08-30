#include "materialsphere.hpp"

#include <physbuzz/graphics/transfer.hpp>
#include <physbuzz/physics/collision.hpp>
#include <physbuzz/render/defines.hpp>
#include <physbuzz/shapes/sphere.hpp>

template <>
Physbuzz::ObjectID ObjectBuilder::create(Physbuzz::Scene &scene, Physbuzz::ObjectID object, MaterialSphere &info) {
    // scale unit sphere for radius
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

    // naming
    IdentifiableComponent identifier = {
        .name = std::format("Material ({})", info.resources.material.getIdentifier()),
    };

    scene.setComponent(object, info.sphere, identifier, info.resources, render);

    return object;
}
