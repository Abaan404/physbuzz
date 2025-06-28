#include "model.hpp"

#include <glm/ext/scalar_constants.hpp>
#include <physbuzz/physics/collision.hpp>
#include <physbuzz/render/shadow.hpp>
#include <physbuzz/resources/manager.hpp>

template <>
Physbuzz::ObjectID ObjectBuilder::create(Physbuzz::ObjectID object, Model &info) {
    // setup rendering
    info.transform.update();
    Physbuzz::ShadowComponent shadow;
    Physbuzz::RenderComponent render = {
        .transform = info.transform,
        .model = info.model.resource,
        .pipeline = info.resources.pipeline,
    };

    scene->setComponent(object, info.model, info.identifier, info.resources, render, shadow);

    return object;
}
