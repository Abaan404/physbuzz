#include "lightdirectional.hpp"

template <>
Physbuzz::ObjectID ObjectBuilder::create(Physbuzz::ObjectID object, LightDirectional &info) {
    info.directionalLight.direction = glm::normalize(info.directionalLight.direction);


    RebuildableComponent rebuilder = {
        .rebuild = [](ObjectBuilder &builder, Physbuzz::ObjectID object) {
            if (!builder.scene->containsComponent<Physbuzz::DirectionalLightComponent, IdentifiableComponent>(object)) {
                Physbuzz::Logger::ERROR("[RebuildableComponent] Cannot rebuild object with id '{}' with missing core components.", object);
                return;
            }

            const auto [directional, identifier] = builder.scene->getComponent<Physbuzz::DirectionalLightComponent, IdentifiableComponent>(object);

            LightDirectional info = {
                .directionalLight = directional,
                .identifier = identifier,
            };

            builder.create(object, info);
        },
    };

    scene->setComponent(object, info.directionalLight, info.identifier, rebuilder);

    return object;
}
