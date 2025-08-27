#include "cube.hpp"

#include <physbuzz/physics/collision.hpp>
#include <physbuzz/physics/dynamics.hpp>
#include <physbuzz/render/model.hpp>
#include <physbuzz/render/renderers/forward.hpp>
#include <physbuzz/render/shadow.hpp>

template <>
Physbuzz::ObjectID ObjectBuilder::create(Physbuzz::Scene &scene, Physbuzz::ObjectID object, Cube &info) {
    // generate mesh
    glm::vec3 min = glm::vec3(-info.cube.width / 2.0f, -info.cube.height / 2.0f, -info.cube.length / 2.0f);
    glm::vec3 max = glm::vec3(info.cube.width / 2.0f, info.cube.height / 2.0f, info.cube.length / 2.0f);

    Physbuzz::Mesh mesh = Physbuzz::Mesh::Info<Physbuzz::Model::Vertex>{
        .transfer = scene.getSystem<Physbuzz::Transfer>(),
        .vertices = {
            {{min.x, min.y, min.z}, {-1.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 0.0f}},
            {{min.x, min.y, max.z}, {-1.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 1.0f}, {1.0f, 0.0f}},
            {{min.x, max.y, max.z}, {-1.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}},
            {{min.x, max.y, min.z}, {-1.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f}},

            {{max.x, min.y, max.z}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f, -1.0f}, {0.0f, 0.0f}},
            {{max.x, min.y, min.z}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f, -1.0f}, {1.0f, 0.0f}},
            {{max.x, max.y, min.z}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f, -1.0f}, {1.0f, 1.0f}},
            {{max.x, max.y, max.z}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f, -1.0f}, {0.0f, 1.0f}},

            {{max.x, min.y, min.z}, {0.0f, 0.0f, -1.0f}, {-1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}},
            {{min.x, min.y, min.z}, {0.0f, 0.0f, -1.0f}, {-1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}},
            {{min.x, max.y, min.z}, {0.0f, 0.0f, -1.0f}, {-1.0f, 0.0f, 0.0f}, {1.0f, 1.0f}},
            {{max.x, max.y, min.z}, {0.0f, 0.0f, -1.0f}, {-1.0f, 0.0f, 0.0f}, {0.0f, 1.0f}},

            {{min.x, min.y, max.z}, {0.0f, 0.0f, 1.0f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}},
            {{max.x, min.y, max.z}, {0.0f, 0.0f, 1.0f}, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}},
            {{max.x, max.y, max.z}, {0.0f, 0.0f, 1.0f}, {1.0f, 0.0f, 0.0f}, {1.0f, 1.0f}},
            {{min.x, max.y, max.z}, {0.0f, 0.0f, 1.0f}, {1.0f, 0.0f, 0.0f}, {0.0f, 1.0f}},

            {{min.x, min.y, min.z}, {0.0f, -1.0f, 0.0f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}},
            {{max.x, min.y, min.z}, {0.0f, -1.0f, 0.0f}, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}},
            {{max.x, min.y, max.z}, {0.0f, -1.0f, 0.0f}, {1.0f, 0.0f, 0.0f}, {1.0f, 1.0f}},
            {{min.x, min.y, max.z}, {0.0f, -1.0f, 0.0f}, {1.0f, 0.0f, 0.0f}, {0.0f, 1.0f}},

            {{max.x, max.y, min.z}, {0.0f, 1.0f, 0.0f}, {-1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}},
            {{min.x, max.y, min.z}, {0.0f, 1.0f, 0.0f}, {-1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}},
            {{min.x, max.y, max.z}, {0.0f, 1.0f, 0.0f}, {-1.0f, 0.0f, 0.0f}, {1.0f, 1.0f}},
            {{max.x, max.y, max.z}, {0.0f, 1.0f, 0.0f}, {-1.0f, 0.0f, 0.0f}, {0.0f, 1.0f}},
        },
        .indices = {0, 1, 2, 2, 3, 0, 4, 5, 6, 6, 7, 4, 8, 9, 10, 10, 11, 8, 12, 13, 14, 14, 15, 12, 16, 17, 18, 18, 19, 16, 20, 21, 22, 22, 23, 20},
    };

    // create model
    std::string modelName = std::format("cube_{}", object);
    Physbuzz::ResourceRegistry<Physbuzz::Model>::insert(
        modelName,
        {{
            .path = {},
            .meshes = {{mesh, {}}},
            .textures = info.resources.textures,
        }});

    // setup rendering
    info.transform.update();
    Physbuzz::ShadowComponent shadow;
    Physbuzz::RenderComponent render = {
        .transform = info.transform,
        .model = modelName,
    };

    Physbuzz::ForwardRenderComponent forward = {};
    Physbuzz::DeferredRenderComponent deferred = {};

    // create a rebuild callback
    RebuildableComponent rebuilder = {
        .rebuild = [](Physbuzz::Scene &scene, Physbuzz::ObjectID object) {
            if (!scene.containsComponent<CubeComponent, IdentifiableComponent, ResourceComponent, Physbuzz::RenderComponent>(object)) {
                Physbuzz::Logger::ERROR("[RebuildableComponent] Cannot rebuild object with id '{}' with missing core components.", object);
                return;
            }

            const auto [cube, identifier, resources, render] = scene.getComponent<CubeComponent, IdentifiableComponent, ResourceComponent, Physbuzz::RenderComponent>(object);

            Cube info = {
                // .body = object.getComponent<Physbuzz::RigidBodyComponent>(),
                .cube = cube,
                .transform = render.transform,
                .identifier = identifier,
                .resources = resources,
                .hasPhysics = scene.containsComponent<Physbuzz::RigidBodyComponent>(object),
            };

            if (info.hasPhysics) {
                // info.body = scene.getComponent<Physbuzz::RigidBodyComponent>(object);
            }

            ObjectBuilder::create(scene, object, info);
        },
    };

    scene.setComponent(object, info.cube, info.identifier, info.resources, render, forward, deferred, shadow, rebuilder);

    // if (info.hasPhysics) {
    //     // generate bounding box
    //     Physbuzz::AABBComponent aabb = Physbuzz::AABBComponent(render);
    //     scene.setComponent(object, aabb);
    // }

    return object;
}
