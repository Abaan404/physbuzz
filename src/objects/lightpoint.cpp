#include "lightpoint.hpp"

#include <physbuzz/render/renderers/defines.hpp>
#include <physbuzz/shapes/sphere.hpp>

template <>
Physbuzz::ObjectID ObjectBuilder::create(Physbuzz::Scene &scene, Physbuzz::ObjectID object, LightPoint &info) {
    // scale unit sphere
    info.transform.scale = {info.sphere.radius, info.sphere.radius, info.sphere.radius};
    Physbuzz::Builtin::ModelSphere::build(scene.getSystem<Physbuzz::Transfer>());

    // setup rendering
    info.transform.update();
    Physbuzz::RenderComponent render = {
        .transform = info.transform,
        .model = Physbuzz::Model::Info{
            .meshes = {
                {
                    .materialIdx = 0,
                    .resource = Physbuzz::Builtin::ModelSphere::Resource,
                },
            },
            .materials = {},
        },
    };

    // point light matches this object's transform
    info.pointLight.position = {
        info.transform.position.x,
        info.transform.position.y,
        info.transform.position.z,
    };

    // create a rebuild callback
    RebuildableComponent rebuilder = {
        .rebuild = [](Physbuzz::Scene &scene, Physbuzz::ObjectID object) {
            if (!scene.containsComponent<RadialComponent, IdentifiableComponent, ResourceComponent, Physbuzz::RenderComponent>(object)) {
                Physbuzz::Logger::ERROR("[RebuildableComponent] Cannot rebuild object with id '{}' with missing core components.", object);
                return;
            }

            const auto [sphere, identifier, resources, render, pointLight] = scene.getComponent<RadialComponent, IdentifiableComponent, ResourceComponent, Physbuzz::RenderComponent, Physbuzz::PointLightComponent>(object);

            LightPoint info = {
                .sphere = sphere,
                .transform = render.transform,
                .identifier = identifier,
                .resources = resources,
                .pointLight = pointLight,
            };

            ObjectBuilder::create(scene, object, info);
        },
    };

    scene.setComponent(object, info.sphere, info.identifier, info.resources, info.pointLight, render, rebuilder);

    return object;
}
