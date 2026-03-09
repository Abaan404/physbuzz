#include "lightpoint.hpp"

#include <physbuzz/graphics/transfer.hpp>
#include <physbuzz/render/defines.hpp>
#include <physbuzz/shapes/sphere.hpp>

template <>
Physbuzz::ObjectID ObjectBuilder::create(Physbuzz::Scene &scene, Physbuzz::ObjectID object, LightPoint &info) {
    // scale unit sphere
    info.transform.setScale({info.sphere.radius, info.sphere.radius, info.sphere.radius});

    // setup rendering
    Physbuzz::RenderComponent render = {
        .transform = info.transform,
        .model = {{
            .meshes = {
                {
                    .material = info.resources.material,
                    .mesh = Physbuzz::Builtin::ModelSphere::Resource,
                },
            },
        }},
    };

    // point light matches this object's transform
    info.pointLight.setPosition({
        info.transform.getInfo().position.x,
        info.transform.getInfo().position.y,
        info.transform.getInfo().position.z,
    });

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
