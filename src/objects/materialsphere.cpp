#include "materialsphere.hpp"

#include <physbuzz/graphics/transfer.hpp>
#include <physbuzz/physics/collision.hpp>
#include <physbuzz/render/defines.hpp>
#include <physbuzz/shapes/sphere.hpp>

template <>
Physbuzz::ObjectID ObjectBuilder::create(Physbuzz::Scene &scene, Physbuzz::ObjectID object, MaterialSphere &info) {
    // scale unit sphere for radius
    info.transform.setScale({info.sphere.radius, info.sphere.radius, info.sphere.radius});

    // setup rendering
    Physbuzz::RenderComponent render = {
        .transform = info.transform,
        .model = {{
            .mesh = Physbuzz::Builtin::ModelSphere::Resource,
            .materials = {info.resources.material},
            .submeshMaterialIndices = {0},
        }},
    };

    // naming
    IdentifiableComponent identifier = {
        .name = std::format("Material ({})", info.resources.material.getIdentifier()),
    };

    // create a rebuild callback
    RebuildableComponent rebuilder = {
        .rebuild = [](Physbuzz::Scene &scene, Physbuzz::ObjectID object) {
            if (!scene.containsComponent<RadialComponent, IdentifiableComponent, ResourceComponent, Physbuzz::RenderComponent>(object)) {
                Physbuzz::Logger::ERROR("[RebuildableComponent] Cannot rebuild object with id '{}' with missing core components.", object);
                return;
            }

            const auto [materialsphere, identifier, resources, render] = scene.getComponent<RadialComponent, IdentifiableComponent, ResourceComponent, Physbuzz::RenderComponent>(object);

            MaterialSphere info = {
                .sphere = materialsphere,
                .transform = render.transform,
                .resources = resources,
            };

            ObjectBuilder::create(scene, object, info);
        },
    };

    scene.setComponent(object, info.sphere, identifier, info.resources, render, rebuilder);

    return object;
}
