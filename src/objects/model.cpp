#include "model.hpp"

#include <physbuzz/render/renderer.hpp>
#include <physbuzz/render/shadow.hpp>
#include <physbuzz/render/renderers/deferred.hpp>
#include <physbuzz/render/renderers/forward.hpp>

template <>
Physbuzz::ObjectID ObjectBuilder::create(Physbuzz::Scene &scene, Physbuzz::ObjectID object, Model &info) {
    // setup rendering
    info.transform.update();
    Physbuzz::ShadowComponent shadow;
    Physbuzz::RenderComponent render = {
        .transform = info.transform,
        .model = info.model.resource,
    };

    Physbuzz::DeferredRenderComponent deferred = {};

    scene.setComponent(object, info.model, info.identifier, info.resources, render, deferred, shadow);

    return object;
}
