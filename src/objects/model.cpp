#include "model.hpp"

#include <physbuzz/graphics/transfer.hpp>
#include <physbuzz/render/shadow.hpp>

template <>
Physbuzz::ObjectID ObjectBuilder::create(Physbuzz::Scene &scene, Physbuzz::ObjectID object, Model &info) {
    // setup rendering
    Physbuzz::RenderComponent render = {
        .model = Physbuzz::Model::Info{
            .mesh = {{}},
            .materials = {{}},
            .submeshMaterialIndices = {},
        },
    };

    Physbuzz::TransformComponent transform = {
        .transform = info.transform,
    };

    render.model.load(info.model.path, scene.getSystem<Physbuzz::Transfer>(), info.model.flipUVs);

    Physbuzz::ShadowComponent shadow = {
        .model = render.model,
    };

    scene.setComponent(object, info.model, info.identifier, render, shadow, transform);

    return object;
}
