#include "cuboid.hpp"

#include <physbuzz/graphics/transfer.hpp>
#include <physbuzz/physics/collision.hpp>
#include <physbuzz/physics/dynamics.hpp>
#include <physbuzz/render/defines.hpp>
#include <physbuzz/render/shadow.hpp>
#include <physbuzz/shapes/cube.hpp>

template <>
Physbuzz::ObjectID ObjectBuilder::create(Physbuzz::Scene &scene, Physbuzz::ObjectID object, Cuboid &info) {
    // scale unit cube for cuboid
    info.transform.setScale({info.cuboid.width, info.cuboid.breadth, info.cuboid.height});

    // setup rendering
    Physbuzz::ShadowComponent shadow;
    Physbuzz::RenderComponent render = {
        .transform = info.transform,
        .model = {{
            .mesh = Physbuzz::Builtin::ModelCube::Resource,
            .materials = {info.resources.material},
            .submeshMaterialIndices = {0},
        }},
    };

    scene.setComponent(object, info.cuboid, info.identifier, info.resources, render, shadow);

    // TODO clean this up
    if (info.hasPhysics) {
        const std::vector<std::byte> &modelIndices = Physbuzz::Builtin::ModelCube::Resource->getIndices();
        std::span<const Physbuzz::Index> indices(
            reinterpret_cast<const Physbuzz::Index *>(modelIndices.data()),
            modelIndices.size() / sizeof(Physbuzz::Index));

        const std::vector<std::byte> &modelVertices = Physbuzz::Builtin::ModelCube::Resource->getVertices();
        std::span<const Physbuzz::Model::Vertex> vertices(
            reinterpret_cast<const Physbuzz::Model::Vertex *>(modelVertices.data()),
            modelVertices.size() / sizeof(Physbuzz::Model::Vertex));

        info.body.geometry.indices = std::vector(indices.begin(), indices.end());

        info.body.geometry.vertices.reserve(vertices.size());
        for (const auto &vertex : vertices) {
            info.body.geometry.vertices.emplace_back<Physbuzz::RigidBodyComponent::Vertex>({
                .position = vertex.position,
                .normal = vertex.normal,
            });
        }

        Physbuzz::RigidBodyComponent body = info.body;

        // generate bounding box
        Physbuzz::BoundingComponent bounding = {
            .aabb = body.getAABB().withTransform(info.transform),
        };

        scene.setComponent(object, bounding, body);
    }

    return object;
}
