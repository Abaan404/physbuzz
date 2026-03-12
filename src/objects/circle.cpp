#include "circle.hpp"

#include <physbuzz/graphics/transfer.hpp>
#include <physbuzz/physics/collision.hpp>
#include <physbuzz/render/defines.hpp>
#include <physbuzz/shapes/circle.hpp>

template <>
Physbuzz::ObjectID ObjectBuilder::create(Physbuzz::Scene &scene, Physbuzz::ObjectID object, Circle &info) {
    // scale unit circle for radius
    info.transform.setScale({info.circle.radius, info.circle.radius, info.circle.radius});

    // setup rendering
    Physbuzz::RenderComponent render = {
        .transform = info.transform,
        .model = {{
            .mesh = Physbuzz::Builtin::ModelCircle::Resource,
            .materials = {info.resources.material},
            .submeshMaterialIndices = {0},
        }},
    };

    // create a rebuild callback
    RebuildableComponent rebuilder = {
        .rebuild = [](Physbuzz::Scene &scene, Physbuzz::ObjectID object) {
            if (!scene.containsComponent<RadialComponent, IdentifiableComponent, ResourceComponent, Physbuzz::RenderComponent>(object)) {
                Physbuzz::Logger::ERROR("[RebuildableComponent] Cannot rebuild object with id '{}' with missing core components.", object);
                return;
            }

            const auto [circle, identifier, resources, render] = scene.getComponent<RadialComponent, IdentifiableComponent, ResourceComponent, Physbuzz::RenderComponent>(object);

            Circle info = {
                .body = {},
                .circle = circle,
                .transform = render.transform,
                .identifier = identifier,
                .resources = resources,
                .hasPhysics = scene.containsComponent<Physbuzz::RigidBodyComponent>(object),
            };

            if (info.hasPhysics) {
                const auto [body] = scene.getComponent<Physbuzz::RigidBodyComponent>(object);
                info.body = body;
            }

            ObjectBuilder::create(scene, object, info);
        },
    };

    scene.setComponent(object, info.circle, info.identifier, info.resources, render, rebuilder);

    // // generate physics info
    // if (info.hasPhysics) {
    //     // build inertia
    //     // Mx = (r*sin(theta))**2 * r).integrate((theta, 0, 2*pi)).integrate((r, 0, a)) * rho
    //     // My = (r*cos(theta))**2 * r).integrate((theta, 0, 2*pi)).integrate((r, 0, a)) * rho
    //     info.body.angular.inertia = info.body.mass * glm::pow(info.circle.radius, 2) / 2.0f;
    //
    //     // generate bounding box
    //     Physbuzz::AABBComponent aabb = Physbuzz::AABBComponent(render);
    //
    //     scene.setComponent(object, info.body, aabb);
    // }

    return object;
}
