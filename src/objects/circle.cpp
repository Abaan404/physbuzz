#include "circle.hpp"

#include <physbuzz/physics/collision.hpp>

template <>
Physbuzz::ObjectID ObjectBuilder::create(Physbuzz::ObjectID object, Circle &info) {
    constexpr Physbuzz::Index MAX_VERTICES = 50;
    constexpr const float angleIncrement = (2.0f * glm::pi<float>()) / MAX_VERTICES;

    Physbuzz::Mesh::Info mesh;

    // calc positions
    mesh.vertices.resize(MAX_VERTICES);
    for (Physbuzz::Index i = 0; i < MAX_VERTICES; i++) {
        float angle = i * angleIncrement;
        mesh.vertices[i].position = info.circle.radius * glm::vec3(glm::cos(angle), glm::sin(angle), 0.0f);
    }

    // calc indices
    for (Physbuzz::Index i = 1; i < MAX_VERTICES - 1; i++) {
        mesh.indices.insert(mesh.indices.end(), {0, i, i + 1});
    }
    mesh.indices.insert(mesh.indices.end(), {0, MAX_VERTICES - 1, 1});

    // calc vertices
    generateTexCoords(mesh);
    generateNormals(mesh);

    // create model
    std::string modelName = std::format("circle_{}", object);
    Physbuzz::ResourceRegistry<Physbuzz::Model>::insert(
        modelName,
        {{
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
        .rebuild = [](ObjectBuilder &builder, Physbuzz::ObjectID object) {
            if (!builder.scene->containsComponent<CircleComponent, IdentifiableComponent, ResourceComponent, Physbuzz::RenderComponent>(object)) {
                Physbuzz::Logger::ERROR("[RebuildableComponent] Cannot rebuild object with id '{}' with missing core components.", object);
                return;
            }
            const auto [circle, identifier, resources, render] = builder.scene->getComponent<CircleComponent, IdentifiableComponent, ResourceComponent, Physbuzz::RenderComponent>(object);

            Circle info = {
                .body = {},
                .circle = circle,
                .transform = render.transform,
                .identifier = identifier,
                .resources = resources,
                .hasPhysics = builder.scene->containsComponent<Physbuzz::RigidBodyComponent>(object),
            };

            if (info.hasPhysics) {
                const auto [body] = builder.scene->getComponent<Physbuzz::RigidBodyComponent>(object);
                info.body = body;
            }

            builder.create(object, info);
        },
    };

    scene->setComponent(object, info.circle, info.identifier, info.resources, render, rebuilder);

    // generate physics info
    if (info.hasPhysics) {
        // build inertia
        // Mx = (r*sin(theta))**2 * r).integrate((theta, 0, 2*pi)).integrate((r, 0, a)) * rho
        // My = (r*cos(theta))**2 * r).integrate((theta, 0, 2*pi)).integrate((r, 0, a)) * rho
        info.body.angular.inertia = info.body.mass * glm::pow(info.circle.radius, 2) / 2.0f;

        // generate bounding box
        Physbuzz::AABBComponent aabb = Physbuzz::AABBComponent(render);

        scene->setComponent(object, info.body, aabb);
    }

    return object;
}
