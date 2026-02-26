#include "lightdirectional.hpp"

template <>
Physbuzz::ObjectID ObjectBuilder::create(Physbuzz::Scene &scene, Physbuzz::ObjectID object, LightDirectional &info) {
    RebuildableComponent rebuilder = {
        .rebuild = [](Physbuzz::Scene &scene, Physbuzz::ObjectID object) {
            if (!scene.containsComponent<Physbuzz::DirectionalLightComponent, IdentifiableComponent>(object)) {
                Physbuzz::Logger::ERROR("[RebuildableComponent] Cannot rebuild object with id '{}' with missing core components.", object);
                return;
            }

            const auto [directional, identifier] = scene.getComponent<Physbuzz::DirectionalLightComponent, IdentifiableComponent>(object);

            LightDirectional info = {
                .directionalLight = directional,
                .identifier = identifier,
            };

            ObjectBuilder::create(scene, object, info);
        },
    };

    scene.setComponent(object, info.directionalLight, info.identifier, rebuilder);

    return object;
}
