#include "model.hpp"

#include <physbuzz/render/renderer.hpp>
#include <physbuzz/render/renderers/deferred.hpp>
#include <physbuzz/render/renderers/forward.hpp>
#include <physbuzz/render/shadow.hpp>

template <>
Physbuzz::ObjectID ObjectBuilder::create(Physbuzz::Scene &scene, Physbuzz::ObjectID object, Model &info) {
    // setup rendering
    info.transform.update();
    Physbuzz::ShadowComponent shadow;
    Physbuzz::RenderComponent render = {
        .transform = info.transform,
        .model = {{}},
    };

    render.model.load(info.model.path, scene.getSystem<Physbuzz::Transfer>());

    scene.setComponent(object, info.model, info.identifier, render, shadow);

    return object;
}
