#include "model.hpp"

#include <physbuzz/graphics/transfer.hpp>
#include <physbuzz/render/shadow.hpp>
#include <physbuzz/shapes/cube.hpp>

template <>
Physbuzz::ObjectID ObjectBuilder::create(Physbuzz::Scene &scene, Physbuzz::ObjectID object, Model &info) {
    // setup rendering
    Physbuzz::ShadowComponent shadow;
    Physbuzz::RenderComponent render = {
        .transform = info.transform,
        .model = Physbuzz::Model::Info{
            .mesh = Physbuzz::Builtin::ModelCube::Resource,
            .materials = {{"default"}},
            .submeshMaterialIndices = {0},
        },
    };

    render.model.load(info.model.path, scene.getSystem<Physbuzz::Transfer>(), info.model.flipTextureVertically);

    scene.setComponent(object, info.model, info.identifier, render, shadow);

    return object;
}
