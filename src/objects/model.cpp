#include "model.hpp"

#include <glm/ext/scalar_constants.hpp>
#include <physbuzz/physics/collision.hpp>
#include <physbuzz/resources/manager.hpp>

template <>
Physbuzz::ObjectID ObjectBuilder::create(Physbuzz::ObjectID object, Model &info) {
    // setup rendering
    Physbuzz::ModelComponent render = {
        .model = info.model.id,
        .pipeline = info.pipeline,
    };

    info.transform.update();

    // create a rebuild callback
    RebuildableComponent rebuilder = {
        .rebuild = [](ObjectBuilder &builder, Physbuzz::ObjectID object) {
            if (!builder.scene->containsComponent<ModelComponent, Physbuzz::TransformComponent, IdentifiableComponent, Physbuzz::ModelComponent>(object)) {
                Physbuzz::Logger::ERROR("[RebuildableComponent] Cannot rebuild object with id '{}' with missing core components.", object);
                return;
            }

            Model info = {
                .model = builder.scene->getComponent<ModelComponent>(object),
                .transform = builder.scene->getComponent<Physbuzz::TransformComponent>(object),
                .identifier = builder.scene->getComponent<IdentifiableComponent>(object),
                .pipeline = builder.scene->getComponent<Physbuzz::ModelComponent>(object).pipeline,
                .hasPhysics = builder.scene->containsComponent<Physbuzz::RigidBodyComponent>(object),
            };

            if (info.hasPhysics) {
                info.body = builder.scene->getComponent<Physbuzz::RigidBodyComponent>(object);
            }

            builder.create(object, info);
        },
    };

    scene->setComponent(object, info.model, info.identifier, info.transform, render, rebuilder);

    return object;
}
