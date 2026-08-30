#include "lightdirectional.hpp"

template <>
Physbuzz::ObjectID ObjectBuilder::create(Physbuzz::Scene &scene, Physbuzz::ObjectID object, LightDirectional &info) {
    scene.setComponent(object, info.directionalLight, info.identifier);

    return object;
}
