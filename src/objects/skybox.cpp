#include "skybox.hpp"

#include <physbuzz/render/renderers/defines.hpp>

struct VertexSkybox {
    glm::vec3 position;

    static Physbuzz::VertexDescription Description;
};

Physbuzz::VertexDescription VertexSkybox::Description = {{
    .attributes = {
        {
            .format = Physbuzz::VertexDescription::Format::eR32G32B32Sfloat,
            .size = sizeof(VertexSkybox::position) / sizeof(decltype(VertexSkybox::position)::value_type),
            .offset = offsetof(VertexSkybox, position),
        },
    },
    .size = sizeof(VertexSkybox),
    .binding = 0,
}};

template <>
Physbuzz::ObjectID ObjectBuilder::create(Physbuzz::Scene &scene, Physbuzz::ObjectID object, Skybox &info) {
    // generate mesh
    constexpr glm::vec3 min = glm::vec3(-2.0f, -2.0f, -2.0f);
    constexpr glm::vec3 max = glm::vec3(2.0f, 2.0f, 2.0f);

    Physbuzz::Mesh::Info<VertexSkybox> mesh = {
        .vertices = {
            {{min.x, min.y, min.z}},
            {{min.x, min.y, max.z}},
            {{min.x, max.y, max.z}},
            {{min.x, max.y, min.z}},

            {{max.x, min.y, max.z}},
            {{max.x, min.y, min.z}},
            {{max.x, max.y, min.z}},
            {{max.x, max.y, max.z}},

            {{max.x, min.y, min.z}},
            {{min.x, min.y, min.z}},
            {{min.x, max.y, min.z}},
            {{max.x, max.y, min.z}},

            {{min.x, min.y, max.z}},
            {{max.x, min.y, max.z}},
            {{max.x, max.y, max.z}},
            {{min.x, max.y, max.z}},

            {{min.x, min.y, min.z}},
            {{max.x, min.y, min.z}},
            {{max.x, min.y, max.z}},
            {{min.x, min.y, max.z}},

            {{max.x, max.y, min.z}},
            {{min.x, max.y, min.z}},
            {{min.x, max.y, max.z}},
            {{max.x, max.y, max.z}},
        },
        .indices = {0, 3, 2, 2, 1, 0, 4, 7, 6, 6, 5, 4, 8, 11, 10, 10, 9, 8, 12, 15, 14, 14, 13, 12, 16, 19, 18, 18, 17, 16, 20, 23, 22, 22, 21, 20},
    };

    std::string modelName = std::format("skybox", object);
    Physbuzz::ResourceRegistry<Physbuzz::Mesh>::insert(modelName, mesh, scene.getSystem<Physbuzz::Transfer>());

    // setup rendering
    info.transform.update();
    Physbuzz::RenderComponent render = {
        .transform = info.transform,
        .model = {{
            .meshes = {},
        }},
    };

    // setup rendering
    scene.setComponent(object, info.skybox, render);

    return object;
}
