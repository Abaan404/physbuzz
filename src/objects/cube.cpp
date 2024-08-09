#include "cube.hpp"

#include <physbuzz/collision.hpp>
#include <physbuzz/shaders.hpp>

template <>
Physbuzz::ObjectID ObjectBuilder::create(Physbuzz::Object &object, Cube &info) {
    // user-defined components
    object.setComponent(info.cube);
    object.setComponent(info.transform);
    object.setComponent(info.identifier);
    object.setComponent(info.resources);

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
        object.setComponent(mesh);

        // generate bounding box
        if (info.isCollidable) {
            Physbuzz::AABBComponent aabb = Physbuzz::AABBComponent(mesh, info.transform);
            object.setComponent(aabb);
        }
    }

    // create a rebuild callback
    {
        RebuildableComponent rebuilder = {
            .rebuild = [](ObjectBuilder &builder, Physbuzz::Object &object) {
                if (object.hasComponent<Physbuzz::Mesh>()) {
                    object.getComponent<Physbuzz::Mesh>().destroy();
                }

                Cube info = {
                    // .body = object.getComponent<Physbuzz::RigidBodyComponent>(),
                    .transform = object.getComponent<Physbuzz::TransformableComponent>(),
                    .cube = object.getComponent<CubeComponent>(),
                    .identifier = object.getComponent<IdentifiableComponent>(),
                    .isCollidable = object.hasComponent<Physbuzz::AABBComponent>(),
                    .isRenderable = object.hasComponent<Physbuzz::Mesh>(),
                };

                object.eraseComponents();
                builder.create(object, info);
            },
        };

        object.setComponent(rebuilder);
    }

    return object.getId();
}
