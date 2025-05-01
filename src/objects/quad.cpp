#include "quad.hpp"

#include <physbuzz/physics/collision.hpp>
#include <physbuzz/resources/manager.hpp>

template <>
Physbuzz::ObjectID ObjectBuilder::create(Physbuzz::ObjectID object, Quad &info) {
    // generate mesh
    glm::vec3 min = glm::vec3(-info.quad.width / 2.0f, -info.quad.height / 2.0f, 0.0f);
    glm::vec3 max = glm::vec3(info.quad.width / 2.0f, info.quad.height / 2.0f, 0.0f);

    Physbuzz::Mesh mesh;

    // calc positions
    std::vector<glm::vec3> positions = {
        {min.x, min.y, 0.0f}, // top-left
        {min.x, max.y, 0.0f}, // top-right
        {max.x, max.y, 0.0f}, // bottom-right
        {max.x, min.y, 0.0f}, // bottom-left
    };

    // calc indices
    mesh.indices = {0, 1, 2, 0, 3, 2};

    mesh.vertices.resize(positions.size());
    for (std::size_t i = 0; i < mesh.vertices.size(); i++) {
        mesh.vertices[i].position = positions[i];
    }

    // calc vertices
    generate2DTexCoords(mesh);
    generate2DNormals(mesh);

    // create model
    std::string model = std::format("quad_{}", object);
    if (Physbuzz::ResourceRegistry::contains<Physbuzz::ModelResource>(model)) {
        Physbuzz::ResourceRegistry::erase<Physbuzz::ModelResource>(model);
    }

    Physbuzz::ResourceRegistry::insert(model, Physbuzz::ModelResource({mesh}, info.textures.texture2D));

    // setup rendering
    Physbuzz::ModelComponent render = {
        .model = model,
        .pipeline = info.pipeline,
    };

    info.transform.update();

    // create a rebuild callback
    RebuildableComponent rebuilder = {
        .rebuild = [](ObjectBuilder &builder, Physbuzz::ObjectID object) {
            if (!builder.scene->containsComponent<QuadComponent, Physbuzz::TransformComponent, IdentifiableComponent, Physbuzz::ModelComponent, TextureResources>(object)) {
                Physbuzz::Logger::ERROR("[RebuildableComponent] Cannot rebuild object with id '{}' with missing core components.", object);
                return;
            }

            Quad info = {
                .quad = builder.scene->getComponent<QuadComponent>(object),
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

    scene->setComponent(object, info.quad, info.identifier, info.textures, info.transform, render, rebuilder);

    // generate bounding box
    if (info.hasPhysics) {
        Physbuzz::AABBComponent aabb = Physbuzz::AABBComponent(mesh, info.transform);
        scene->setComponent(object, aabb);
    }

    // build inertia
    {
        // Mx = (y**2).integrate((x, -a/2, a/2)).integrate((y, -b/2, b/2)) * rho
        // My = (x**2).integrate((x, -a/2, a/2)).integrate((y, -b/2, b/2)) * rho
        info.body.angular.inertia = info.body.mass * (glm::pow(info.quad.width, 2) + glm::pow(info.quad.height, 2)) / 12.0f;

        scene->setComponent(object, info.body);
    }

    return object;
}
