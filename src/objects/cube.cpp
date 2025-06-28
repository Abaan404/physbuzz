#include "cube.hpp"

#include <physbuzz/render/shadow.hpp>
#include <physbuzz/physics/collision.hpp>
#include <physbuzz/physics/dynamics.hpp>
#include <physbuzz/resources/manager.hpp>

template <>
Physbuzz::ObjectID ObjectBuilder::create(Physbuzz::ObjectID object, Cube &info) {
    // generate mesh
    glm::vec3 min = glm::vec3(-info.cube.width / 2.0f, -info.cube.height / 2.0f, -info.cube.length / 2.0f);
    glm::vec3 max = glm::vec3(info.cube.width / 2.0f, info.cube.height / 2.0f, info.cube.length / 2.0f);

    Physbuzz::Mesh mesh = {{
        .vertices = {
            {{min.x, min.y, min.z}, {-1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}},
            {{min.x, min.y, max.z}, {-1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}},
            {{min.x, max.y, max.z}, {-1.0f, 0.0f, 0.0f}, {1.0f, 1.0f}},
            {{min.x, max.y, min.z}, {-1.0f, 0.0f, 0.0f}, {0.0f, 1.0f}},

            {{max.x, min.y, max.z}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}},
            {{max.x, min.y, min.z}, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}},
            {{max.x, max.y, min.z}, {1.0f, 0.0f, 0.0f}, {1.0f, 1.0f}},
            {{max.x, max.y, max.z}, {1.0f, 0.0f, 0.0f}, {0.0f, 1.0f}},

            {{max.x, min.y, min.z}, {0.0f, 0.0f, -1.0f}, {0.0f, 0.0f}},
            {{min.x, min.y, min.z}, {0.0f, 0.0f, -1.0f}, {1.0f, 0.0f}},
            {{min.x, max.y, min.z}, {0.0f, 0.0f, -1.0f}, {1.0f, 1.0f}},
            {{max.x, max.y, min.z}, {0.0f, 0.0f, -1.0f}, {0.0f, 1.0f}},

            {{min.x, min.y, max.z}, {0.0f, 0.0f, 1.0f}, {0.0f, 0.0f}},
            {{max.x, min.y, max.z}, {0.0f, 0.0f, 1.0f}, {1.0f, 0.0f}},
            {{max.x, max.y, max.z}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}},
            {{min.x, max.y, max.z}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f}},

            {{min.x, min.y, min.z}, {0.0f, -1.0f, 0.0f}, {0.0f, 0.0f}},
            {{max.x, min.y, min.z}, {0.0f, -1.0f, 0.0f}, {1.0f, 0.0f}},
            {{max.x, min.y, max.z}, {0.0f, -1.0f, 0.0f}, {1.0f, 1.0f}},
            {{min.x, min.y, max.z}, {0.0f, -1.0f, 0.0f}, {0.0f, 1.0f}},

            {{max.x, max.y, min.z}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f}},
            {{min.x, max.y, min.z}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f}},
            {{min.x, max.y, max.z}, {0.0f, 1.0f, 0.0f}, {1.0f, 1.0f}},
            {{max.x, max.y, max.z}, {0.0f, 1.0f, 0.0f}, {0.0f, 1.0f}},
        },
        .indices = {0, 1, 2, 2, 3, 0, 4, 5, 6, 6, 7, 4, 8, 9, 10, 10, 11, 8, 12, 13, 14, 14, 15, 12, 16, 17, 18, 18, 19, 16, 20, 21, 22, 22, 23, 20},
    }};

    // create model
    std::string modelName = std::format("cube_{}", object);
    Physbuzz::ResourceRegistry<Physbuzz::ModelResource>::insert(
        modelName,
        {{
            .meshes = {{mesh, {}}},
            .textures = info.resources.textures,
        }});

    // setup rendering
    info.transform.update();
    Physbuzz::ShadowComponent shadow;
    Physbuzz::RenderComponent render = {
        .transform = info.transform,
        .model = modelName,
        .pipeline = info.resources.pipeline,
    };

    // create a rebuild callback
    RebuildableComponent rebuilder = {
        .rebuild = [](ObjectBuilder &builder, Physbuzz::ObjectID object) {
            if (!builder.scene->containsComponent<CubeComponent, IdentifiableComponent, ResourceComponent, Physbuzz::RenderComponent>(object)) {
                Physbuzz::Logger::ERROR("[RebuildableComponent] Cannot rebuild object with id '{}' with missing core components.", object);
                return;
            }

            const auto [cube, identifier, resources, render] = builder.scene->getComponent<CubeComponent, IdentifiableComponent, ResourceComponent, Physbuzz::RenderComponent>(object);

            Cube info = {
                // .body = object.getComponent<Physbuzz::RigidBodyComponent>(),
                .cube = cube,
                .transform = render.transform,
                .identifier = identifier,
                .resources = resources,
                .hasPhysics = builder.scene->containsComponent<Physbuzz::RigidBodyComponent>(object),
            };

            if (info.hasPhysics) {
                // info.body = builder.scene->getComponent<Physbuzz::RigidBodyComponent>(object);
            }

            builder.create(object, info);
        },
    };

    scene->setComponent(object, info.cube, info.identifier, info.resources, render, shadow, rebuilder);

    if (info.hasPhysics) {
        // generate bounding box
        Physbuzz::AABBComponent aabb = Physbuzz::AABBComponent(render);
        scene->setComponent(object, aabb);
    }

    return object;
}
