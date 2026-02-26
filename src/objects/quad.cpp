#include "quad.hpp"

#include <physbuzz/graphics/transfer.hpp>
#include <physbuzz/physics/collision.hpp>
#include <physbuzz/render/defines.hpp>
#include <physbuzz/shapes/square.hpp>

template <>
Physbuzz::ObjectID ObjectBuilder::create(Physbuzz::Scene &scene, Physbuzz::ObjectID object, Quad &info) {
    // scale unit square for quad
    info.transform.setScale({info.quad.width, info.quad.height, 1.0f});
    Physbuzz::Builtin::ModelSquare::build(scene.getSystem<Physbuzz::Transfer>());

    // setup rendering
    Physbuzz::RenderComponent render = {
        .transform = info.transform,
        .model = {{
            .meshes = {
                {
                    .material = info.resources.material,
                    .mesh = Physbuzz::Builtin::ModelSquare::Resource,
                },
            },
        }},
    };

    // create a rebuild callback
    RebuildableComponent rebuilder = {
        .rebuild = [](Physbuzz::Scene &scene, Physbuzz::ObjectID object) {
            if (!scene.containsComponent<QuadComponent, IdentifiableComponent, ResourceComponent, Physbuzz::RenderComponent>(object)) {
                Physbuzz::Logger::ERROR("[RebuildableComponent] Cannot rebuild object with id '{}' with missing core components.", object);
                return;
            }

            const auto [quad, identifier, resources, render] = scene.getComponent<QuadComponent, IdentifiableComponent, ResourceComponent, Physbuzz::RenderComponent>(object);

            Quad info = {
                .body = {},
                .quad = quad,
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

    scene.setComponent(object, info.quad, info.identifier, info.resources, render, rebuilder);

    // generate bounding box
    // if (info.hasPhysics) {
    //     Physbuzz::AABBComponent aabb = Physbuzz::AABBComponent(render);
    //     scene.setComponent(object, aabb);
    // }

    // build inertia
    {
        // Mx = (y**2).integrate((x, -a/2, a/2)).integrate((y, -b/2, b/2)) * rho
        // My = (x**2).integrate((x, -a/2, a/2)).integrate((y, -b/2, b/2)) * rho
        info.body.angular.inertia = info.body.mass * (glm::pow(info.quad.width, 2) + glm::pow(info.quad.height, 2)) / 12.0f;

        scene.setComponent(object, info.body);
    }

    return object;
}
