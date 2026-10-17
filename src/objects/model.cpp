#include "model.hpp"

#include <physbuzz/graphics/transfer.hpp>
#include <physbuzz/render/shadow.hpp>

template <>
Physbuzz::ObjectID ObjectBuilder::create(Physbuzz::Scene &scene, Physbuzz::ObjectID object, Model &info) {
    Physbuzz::TransformComponent transform = {
        .transform = info.transform,
    };

    scene.setComponent(object, info.model, info.identifier, transform);

    Physbuzz::RenderComponent render = {
        .model = {{
            .mesh = {{}},
        }},
    };

    Physbuzz::TransferBatch batch = {{}};
    std::optional<Physbuzz::Model::Result> result = render.model.load(info.model.path, batch, info.model.flipUVs);

    if (!result) {
        return object;
    }

    Physbuzz::Geometry<Physbuzz::ShadowRenderer::Vertex> shadowGeometry = {
        .indices = result->mesh.geometry.indices,
    };

    shadowGeometry.vertices.reserve(result->mesh.geometry.vertices.size());
    for (const auto &vertex : result->mesh.geometry.vertices) {
        shadowGeometry.vertices.emplace_back<Physbuzz::ShadowRenderer::Vertex>({
            .position = vertex.position,
        });
    }

    const Physbuzz::Model::Info &modelInfo = render.model.getInfo();
    const Physbuzz::Mesh::Info &meshInfo = modelInfo.mesh->getInfo();

    Physbuzz::Mesh::Info shadowMeshInfo = {
        .description = &Physbuzz::ShadowRenderer::Vertex::Description,
        .vertexCount = meshInfo.vertexCount,
        .indexCount = meshInfo.indexCount,
        .submeshes = meshInfo.submeshes,
    };

    auto shadowMesh = Physbuzz::ResourceRegistry<Physbuzz::Mesh>::insert(
        std::format("{}/shadow", render.model.getInfo().mesh.getName()),
        Physbuzz::Mesh{shadowMeshInfo});

    if (!shadowMesh) {
        return object;
    }

    if (!(*shadowMesh)->write(std::move(shadowGeometry), batch)) {
        return object;
    }

    scene.getSystem<Physbuzz::Transfer>()->submit(batch);

    Physbuzz::ShadowComponent shadow = {
        .model = {{
            .mesh = *shadowMesh,
            .materials = modelInfo.materials,
            .submeshMaterialIndices = modelInfo.submeshMaterialIndices,
        }},
    };

    scene.setComponent(object, render, shadow);

    return object;
}
