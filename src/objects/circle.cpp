#include "circle.hpp"

#include <physbuzz/physics/collision.hpp>

template <>
Physbuzz::ObjectID ObjectBuilder::create(Physbuzz::Scene &scene, Physbuzz::ObjectID object, Circle &info) {
    constexpr Physbuzz::Index MAX_VERTICES = 50;
    constexpr const float angleIncrement = (2.0f * glm::pi<float>()) / MAX_VERTICES;

    // calc positions
    std::vector<glm::vec3> positions = std::vector<glm::vec3>(MAX_VERTICES);
    for (Physbuzz::Index i = 0; i < MAX_VERTICES; i++) {
        float angle = i * angleIncrement;
        positions[i] = info.circle.radius * glm::vec3(glm::cos(angle), glm::sin(angle), 0.0f);
    }

    // calc indices
    std::vector<Physbuzz::Index> indices;
    for (Physbuzz::Index i = 1; i < MAX_VERTICES - 1; i++) {
        indices.insert(indices.end(), {0, i, i + 1});
    }
    indices.insert(indices.end(), {0, MAX_VERTICES - 1, 1});

    // calc normals
    std::vector<glm::vec3> normals = generateNormals(indices, positions);
    std::vector<glm::vec2> texCoords = generateTexCoords(positions);

    Physbuzz::Mesh<Physbuzz::Builtin::VertexDefault::Format>::Info mesh{
        .attribute = {Physbuzz::Builtin::VertexDefault::Resource.getIdentifier()},
        .vertices = std::vector<Physbuzz::Builtin::VertexDefault::Format>(positions.size()),
        .indices = indices,
    };

    mesh.vertices.resize(positions.size());
    for (std::size_t i = 0; i < mesh.vertices.size(); i++) {
        mesh.vertices[i].position = positions[i];
        mesh.vertices[i].normal = normals[i];
        mesh.vertices[i].texCoords = texCoords[i];
    }

    // create model
    std::string modelName = std::format("circle_{}", object);
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
        .pipeline = info.resources.pipeline,
    };

    // create a rebuild callback
    RebuildableComponent rebuilder = {
        .rebuild = [](Physbuzz::Scene &scene, Physbuzz::ObjectID object) {
            if (!scene.containsComponent<CircleComponent, IdentifiableComponent, ResourceComponent, Physbuzz::RenderComponent>(object)) {
                Physbuzz::Logger::ERROR("[RebuildableComponent] Cannot rebuild object with id '{}' with missing core components.", object);
                return;
            }

            const auto [circle, identifier, resources, render] = scene.getComponent<CircleComponent, IdentifiableComponent, ResourceComponent, Physbuzz::RenderComponent>(object);

            Circle info = {
                .body = {},
                .circle = circle,
                .transform = render.transform,
                .identifier = identifier,
                .resources = resources,
                .hasPhysics = scene.containsComponent<Physbuzz::RigidBodyComponent>(object),
            };

            if (info.hasPhysics) {
                const auto [body] = scene.getComponent<Physbuzz::RigidBodyComponent>(object);
                info.body = body;
            }

            ObjectBuilder::create(scene, object, info);
        },
    };

    scene.setComponent(object, info.circle, info.identifier, info.resources, render, rebuilder);

    // generate physics info
    if (info.hasPhysics) {
        // build inertia
        // Mx = (r*sin(theta))**2 * r).integrate((theta, 0, 2*pi)).integrate((r, 0, a)) * rho
        // My = (r*cos(theta))**2 * r).integrate((theta, 0, 2*pi)).integrate((r, 0, a)) * rho
        info.body.angular.inertia = info.body.mass * glm::pow(info.circle.radius, 2) / 2.0f;

        // generate bounding box
        Physbuzz::AABBComponent aabb = Physbuzz::AABBComponent(render);

        scene.setComponent(object, info.body, aabb);
    }

    return object;
}
