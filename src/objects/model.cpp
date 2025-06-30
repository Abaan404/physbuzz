#include "model.hpp"

#include <physbuzz/render/shadow.hpp>

template <>
Physbuzz::ObjectID ObjectBuilder::create(Physbuzz::Scene &scene, Physbuzz::ObjectID object, Model &info) {
    // setup rendering
    info.transform.update();
    Physbuzz::ShadowComponent shadow;
    Physbuzz::RenderComponent render = {
        .transform = info.transform,
        .model = info.model.resource,
        .pipeline = info.resources.pipeline,
    };

    scene.setComponent(object, info.model, info.identifier, info.resources, render, shadow);

    return object;
}
