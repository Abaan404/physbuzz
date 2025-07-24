#include "skybox.hpp"

#include "../resources/vertex/skybox.hpp"
#include <physbuzz/render/renderer.hpp>

template <>
Physbuzz::ObjectID ObjectBuilder::create(Physbuzz::Scene &scene, Physbuzz::ObjectID object, Skybox &info) {
    // generate mesh
    constexpr glm::vec3 min = glm::vec3(-2.0f, -2.0f, -2.0f);
    constexpr glm::vec3 max = glm::vec3(2.0f, 2.0f, 2.0f);

    Physbuzz::Mesh mesh = Physbuzz::Mesh::Info<VertexSkybox>{
        .attribute = {"skybox"},
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

    std::string modelName = std::format("skybox_{}", object);
    Physbuzz::ResourceRegistry<Physbuzz::Model>::insert(
        modelName,
        {{
            .path = {},
            .meshes = {{mesh, {}}},
            .textures = info.resources.textures,
        }});

    // setup rendering
    info.transform.update();
    Physbuzz::RenderComponent render = {
        .transform = info.transform,
        .model = modelName,
    };

    Physbuzz::DeferredRenderComponent::ForwardPass deferredForward = {
        .pipeline = info.resources.pipeline,
    };

    Physbuzz::ForwardRenderComponent forward = {
        .pipeline = info.resources.pipeline,
    };

    // setup rendering
    scene.setComponent(object, info.resources, info.skybox, render, forward, deferredForward);

    return object;
}
