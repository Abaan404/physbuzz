#include "model.hpp"

#include <physbuzz/graphics/transfer.hpp>
#include <physbuzz/render/shadow.hpp>

template <>
Physbuzz::ObjectID ObjectBuilder::create(Physbuzz::Scene &scene, Physbuzz::ObjectID object, Model &info) {
    // setup rendering
    Physbuzz::ShadowComponent shadow;
    Physbuzz::RenderComponent render = {
        .transform = info.transform,
        .model = Physbuzz::Model::Info{
            .mesh = {{}},
            .materials = {{}},
            .submeshMaterialIndices = {},
        },
    };

    render.model.load(info.model.path, scene.getSystem<Physbuzz::Transfer>(), info.model.flipUVs);

    scene.setComponent(object, info.model, info.identifier, render, shadow);

    return object;
}
