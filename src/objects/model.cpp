#include "model.hpp"

#include <glm/ext/scalar_constants.hpp>
#include <physbuzz/physics/collision.hpp>
#include <physbuzz/resources/manager.hpp>

template <>
Physbuzz::ObjectID ObjectBuilder::create(Physbuzz::ObjectID object, Model &info) {
    // setup rendering
    Physbuzz::ModelComponent render = {
        .model = info.model.id,
    };

    info.transform.update();

    scene->setComponent(object, info.model, info.identifier, info.transform, info.shader, render);

    return object;
}
