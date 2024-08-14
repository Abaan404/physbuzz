#include "cube.hpp"

#include <physbuzz/physics/collision.hpp>
#include <physbuzz/render/shaders.hpp>

template <>
Physbuzz::ObjectID ObjectBuilder::create(Physbuzz::ObjectID object, Cube &info) {
    // user-defined components
    scene->setComponent(object, info.cube, info.transform, info.identifier, info.resources);

    // generate mesh
    if (info.isRenderable) {
        glm::vec3 min = glm::vec3(-info.cube.width / 2.0f, -info.cube.height / 2.0f, -info.cube.length / 2.0f);
        glm::vec3 max = glm::vec3(info.cube.width / 2.0f, info.cube.height / 2.0f, info.cube.length / 2.0f);

        Physbuzz::Mesh mesh;
        mesh.vertices.resize(24);

        // this hurts to look at but probably the best solution
        mesh.vertices = {
            {{min.x, min.y, max.z}, {0.0f, 0.0f, 1.0f}, {0.0f, 0.0f}},
            {{max.x, min.y, max.z}, {0.0f, 0.0f, 1.0f}, {1.0f, 0.0f}},
            {{max.x, max.y, max.z}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}},
            {{min.x, max.y, max.z}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f}},

            {{min.x, min.y, min.z}, {0.0f, 0.0f, -1.0f}, {1.0f, 0.0f}},
            {{max.x, min.y, min.z}, {0.0f, 0.0f, -1.0f}, {0.0f, 0.0f}},
            {{max.x, max.y, min.z}, {0.0f, 0.0f, -1.0f}, {0.0f, 1.0f}},
            {{min.x, max.y, min.z}, {0.0f, 0.0f, -1.0f}, {1.0f, 1.0f}},

            {{min.x, min.y, min.z}, {-1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}},
            {{min.x, min.y, max.z}, {-1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}},
            {{min.x, max.y, max.z}, {-1.0f, 0.0f, 0.0f}, {1.0f, 1.0f}},
            {{min.x, max.y, min.z}, {-1.0f, 0.0f, 0.0f}, {0.0f, 1.0f}},

            {{max.x, min.y, min.z}, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}},
            {{max.x, min.y, max.z}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}},
            {{max.x, max.y, max.z}, {1.0f, 0.0f, 0.0f}, {0.0f, 1.0f}},
            {{max.x, max.y, min.z}, {1.0f, 0.0f, 0.0f}, {1.0f, 1.0f}},

            {{min.x, max.y, min.z}, {0.0f, 1.0f, 0.0f}, {0.0f, 1.0f}},
            {{min.x, max.y, max.z}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f}},
            {{max.x, max.y, max.z}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f}},
            {{max.x, max.y, min.z}, {0.0f, 1.0f, 0.0f}, {1.0f, 1.0f}},

            {{min.x, min.y, min.z}, {0.0f, -1.0f, 0.0f}, {1.0f, 1.0f}},
            {{min.x, min.y, max.z}, {0.0f, -1.0f, 0.0f}, {1.0f, 0.0f}},
            {{max.x, min.y, max.z}, {0.0f, -1.0f, 0.0f}, {0.0f, 0.0f}},
            {{max.x, min.y, min.z}, {0.0f, -1.0f, 0.0f}, {0.0f, 1.0f}},
        };

        // calc indices
        mesh.indices = {
            {0, 1, 2},
            {2, 3, 0},
            {4, 5, 6},
            {6, 7, 4},
            {8, 9, 10},
            {10, 11, 8},
            {12, 13, 14},
            {14, 15, 12},
            {16, 17, 18},
            {18, 19, 16},
            {20, 21, 22},
            {22, 23, 20},
        };

        mesh.build();
        scene->setComponent(object, mesh);

        // generate bounding box
        if (info.isCollidable) {
            Physbuzz::AABBComponent aabb = Physbuzz::AABBComponent(mesh, info.transform);
            scene->setComponent(object, aabb);
        }
    }

    // create a rebuild callback
    {
        RebuildableComponent rebuilder = {
            .rebuild = [](ObjectBuilder &builder, Physbuzz::ObjectID object) {
                Cube info = {
                    // .body = object.getComponent<Physbuzz::RigidBodyComponent>(),
                    .transform = builder.scene->getComponent<Physbuzz::TransformableComponent>(object),
                    .cube = builder.scene->getComponent<CubeComponent>(object),
                    .identifier = builder.scene->getComponent<IdentifiableComponent>(object),
                    .isCollidable = builder.scene->containsComponent<Physbuzz::AABBComponent>(object),
                    .isRenderable = builder.scene->containsComponent<Physbuzz::Mesh>(object),
                };

                builder.create(object, info);
            },
        };

        scene->setComponent(object, rebuilder);
    }

    return object;
}
