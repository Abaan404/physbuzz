#include "skybox.hpp"

#include <physbuzz/resources/manager.hpp>

template <>
Physbuzz::ObjectID ObjectBuilder::create(Physbuzz::ObjectID object, Skybox &info) {
    Physbuzz::Mesh mesh;
    // generate mesh
    constexpr glm::vec3 min = glm::vec3(-2.0f, -2.0f, -2.0f);
    constexpr glm::vec3 max = glm::vec3(2.0f, 2.0f, 2.0f);

    mesh.vertices = {
        {{min.x, min.y, max.z}, {}, {}},
        {{max.x, min.y, max.z}, {}, {}},
        {{max.x, max.y, max.z}, {}, {}},
        {{min.x, max.y, max.z}, {}, {}},
                              
        {{min.x, min.y, min.z}, {}, {}},
        {{max.x, min.y, min.z}, {}, {}},
        {{max.x, max.y, min.z}, {}, {}},
        {{min.x, max.y, min.z}, {}, {}},
                              
        {{min.x, min.y, min.z}, {}, {}},
        {{min.x, min.y, max.z}, {}, {}},
        {{min.x, max.y, max.z}, {}, {}},
        {{min.x, max.y, min.z}, {}, {}},
                              
        {{max.x, min.y, min.z}, {}, {}},
        {{max.x, min.y, max.z}, {}, {}},
        {{max.x, max.y, max.z}, {}, {}},
        {{max.x, max.y, min.z}, {}, {}},
                              
        {{min.x, max.y, min.z}, {}, {}},
        {{min.x, max.y, max.z}, {}, {}},
        {{max.x, max.y, max.z}, {}, {}},
        {{max.x, max.y, min.z}, {}, {}},
                              
        {{min.x, min.y, min.z}, {}, {}},
        {{min.x, min.y, max.z}, {}, {}},
        {{max.x, min.y, max.z}, {}, {}},
        {{max.x, min.y, min.z}, {}, {}},
    };

    // calc indices
    mesh.indices = {0, 1, 2, 2, 3, 0, 4, 5, 6, 6, 7, 4, 8, 9, 10, 10, 11, 8, 12, 13, 14, 14, 15, 12, 16, 17, 18, 18, 19, 16, 20, 21, 22, 22, 23, 20};

    std::string model = std::format("skybox_{}", object);
    Physbuzz::ResourceRegistry::insert(model, Physbuzz::ModelResource({mesh}));

    // setup rendering
    Physbuzz::ModelComponent render = {
        .model = model,
    };

    info.transform.update();

    // setup rendering
    scene->setComponent(object, info.shader, info.skybox, info.transform, render);

    return object;
}
