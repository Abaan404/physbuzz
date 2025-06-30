#include "quad.hpp"

#include <physbuzz/physics/collision.hpp>

template <>
Physbuzz::ObjectID ObjectBuilder::create(Physbuzz::Scene &scene, Physbuzz::ObjectID object, Quad &info) {
    // generate mesh
    glm::vec3 min = glm::vec3(-info.quad.width / 2.0f, -info.quad.height / 2.0f, 0.0f);
    glm::vec3 max = glm::vec3(info.quad.width / 2.0f, info.quad.height / 2.0f, 0.0f);

    Physbuzz::Mesh::Info mesh = {
        .vertices = {
            {{min.x, min.y, 0.0f}, {}, {}}, // top-left
            {{min.x, max.y, 0.0f}, {}, {}}, // top-right
            {{max.x, max.y, 0.0f}, {}, {}}, // bottom-right
            {{max.x, min.y, 0.0f}, {}, {}}, // bottom-left
        },
        .indices = {0, 1, 2, 2, 3, 0},
    };

    generateTexCoords(mesh);
    generateNormals(mesh);

    // create model
    std::string modelName = std::format("quad_{}", object);
    Physbuzz::ResourceRegistry<Physbuzz::Model>::insert(
        modelName,
        {{
            .meshes = {{mesh, {}}},
            .textures = info.resources.textures,
        }});

    // setup rendering
    info.transform.update();
    Physbuzz::RenderComponent render = {
        .transform = info.transform,
        .model = modelName,
        .pipeline = info.resources.pipeline,
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
    if (info.hasPhysics) {
        Physbuzz::AABBComponent aabb = Physbuzz::AABBComponent(render);
        scene.setComponent(object, aabb);
    }

    // build inertia
    {
        // Mx = (y**2).integrate((x, -a/2, a/2)).integrate((y, -b/2, b/2)) * rho
        // My = (x**2).integrate((x, -a/2, a/2)).integrate((y, -b/2, b/2)) * rho
        info.body.angular.inertia = info.body.mass * (glm::pow(info.quad.width, 2) + glm::pow(info.quad.height, 2)) / 12.0f;

        scene.setComponent(object, info.body);
    }

    return object;
}
