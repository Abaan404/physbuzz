#include "model.hpp"

#include <glm/ext/scalar_constants.hpp>
#include <physbuzz/physics/collision.hpp>
#include <physbuzz/resources/manager.hpp>

template <>
Physbuzz::ObjectID ObjectBuilder::create(Physbuzz::ObjectID object, Model &info) {
    // setup rendering
    Physbuzz::ModelComponent render = {
        .model = info.model.resource,
    };

    info.transform.update();

    scene->setComponent(object, info.model, info.identifier, info.transform, info.resources, render);

    return object;
}
