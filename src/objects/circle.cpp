#include "circle.hpp"

#include <glm/ext/scalar_constants.hpp>
#include <physbuzz/physics/collision.hpp>
#include <physbuzz/resources/manager.hpp>

template <>
Physbuzz::ObjectID ObjectBuilder::create(Physbuzz::ObjectID object, Circle &info) {
    constexpr Physbuzz::Index MAX_VERTICES = 50;
    constexpr const float angleIncrement = (2.0f * glm::pi<float>()) / MAX_VERTICES;

    Physbuzz::Mesh mesh;
    mesh.vertices.resize(MAX_VERTICES);

    // calc positions
    for (int i = 0; i < MAX_VERTICES; i++) {
        float angle = i * angleIncrement;
        mesh.vertices[i].position = info.circle.radius * glm::vec3(glm::cos(angle), glm::sin(angle), 0.0f);
    }

    // calc indices
    for (Physbuzz::Index i = 1; i < MAX_VERTICES - 1; i++) {
        mesh.indices.insert(mesh.indices.end(), {0, i, i + 1});
    }
    mesh.indices.insert(mesh.indices.end(), {0, MAX_VERTICES - 1, 1});

    // calc vertices
    generate2DTexCoords(mesh);
    generate2DNormals(mesh);

    // add textures
    mesh.textures = info.textures.texture2D;

    // create model
    std::string model = std::format("circle_{}", object);
    Physbuzz::ResourceRegistry::insert(model, Physbuzz::ModelResource({mesh}));

    // setup rendering
    Physbuzz::ModelComponent render = {
        .model = model,
        .pipeline = info.pipeline,
    };

    info.transform.update();

    // create a rebuild callback
    RebuildableComponent rebuilder = {
        .rebuild = [](ObjectBuilder &builder, Physbuzz::ObjectID object) {
            if (!builder.scene->containsComponent<CircleComponent, Physbuzz::TransformComponent, IdentifiableComponent, Physbuzz::ModelComponent, TextureResources>(object)) {
                Physbuzz::Logger::ERROR("[RebuildableComponent] Cannot rebuild object with id '{}' with missing core components.", object);
                return;
            }

            Circle info = {
                .circle = builder.scene->getComponent<CircleComponent>(object),
                .transform = builder.scene->getComponent<Physbuzz::TransformComponent>(object),
                .identifier = builder.scene->getComponent<IdentifiableComponent>(object),
                .pipeline = builder.scene->getComponent<Physbuzz::ModelComponent>(object).pipeline,
                .textures = builder.scene->getComponent<TextureResources>(object),
                .hasPhysics = builder.scene->containsComponent<Physbuzz::RigidBodyComponent>(object),
            };

            if (info.hasPhysics) {
                info.body = builder.scene->getComponent<Physbuzz::RigidBodyComponent>(object);
            }

            builder.create(object, info);
        },
    };

    scene->setComponent(object, info.circle, info.identifier, info.textures, info.transform, render, rebuilder);

    // generate physics info
    if (info.hasPhysics) {
        // build inertia
        // Mx = (r*sin(theta))**2 * r).integrate((theta, 0, 2*pi)).integrate((r, 0, a)) * rho
        // My = (r*cos(theta))**2 * r).integrate((theta, 0, 2*pi)).integrate((r, 0, a)) * rho
        info.body.angular.inertia = info.body.mass * glm::pow(info.circle.radius, 2) / 2.0f;

        // generate bounding box
        Physbuzz::AABBComponent aabb = Physbuzz::AABBComponent(mesh, info.transform);

        scene->setComponent(object, info.body, aabb);
    }

    return object;
}
