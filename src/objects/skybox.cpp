#include "skybox.hpp"

#include <physbuzz/resources/manager.hpp>

template <>
Physbuzz::ObjectID ObjectBuilder::create(Physbuzz::ObjectID object, Skybox &info) {
    Physbuzz::Mesh mesh;
    // generate mesh
    constexpr glm::vec3 min = glm::vec3(-2.0f, -2.0f, -2.0f);
    constexpr glm::vec3 max = glm::vec3(2.0f, 2.0f, 2.0f);

    mesh.vertices = {
        {{min.x, min.y, min.z}, {}, {}},
        {{min.x, min.y, max.z}, {}, {}},
        {{min.x, max.y, max.z}, {}, {}},
        {{min.x, max.y, min.z}, {}, {}},

        {{max.x, min.y, max.z}, {}, {}},
        {{max.x, min.y, min.z}, {}, {}},
        {{max.x, max.y, min.z}, {}, {}},
        {{max.x, max.y, max.z}, {}, {}},

        {{max.x, min.y, min.z}, {}, {}},
        {{min.x, min.y, min.z}, {}, {}},
        {{min.x, max.y, min.z}, {}, {}},
        {{max.x, max.y, min.z}, {}, {}},

        {{min.x, min.y, max.z}, {}, {}},
        {{max.x, min.y, max.z}, {}, {}},
        {{max.x, max.y, max.z}, {}, {}},
        {{min.x, max.y, max.z}, {}, {}},

        {{min.x, min.y, min.z}, {}, {}},
        {{max.x, min.y, min.z}, {}, {}},
        {{max.x, min.y, max.z}, {}, {}},
        {{min.x, min.y, max.z}, {}, {}},

        {{max.x, max.y, min.z}, {}, {}},
        {{min.x, max.y, min.z}, {}, {}},
        {{min.x, max.y, max.z}, {}, {}},
        {{max.x, max.y, max.z}, {}, {}},
    };

    // calc indices
    mesh.indices = {0, 3, 2, 2, 1, 0, 4, 7, 6, 6, 5, 4, 8, 11, 10, 10, 9, 8, 12, 15, 14, 14, 13, 12, 16, 19, 18, 18, 17, 16, 20, 23, 22, 22, 21, 20};

    std::string model = std::format("skybox_{}", object);
    Physbuzz::ResourceRegistry<Physbuzz::ModelResource>::insert(model, {{mesh}});

    // setup rendering
    Physbuzz::ModelComponent render = {
        .model = model,
    };

    info.transform.update();

    // setup rendering
    scene->setComponent(object, info.resources, info.skybox, info.transform, render);

    return object;
}
