#include "wall.hpp"

#include "quad.hpp"
#include <glm/glm.hpp>
#include <physbuzz/physics/collision.hpp>

template <>
Physbuzz::ObjectID ObjectBuilder::create(Physbuzz::ObjectID object, Wall &info) {
    // user-defined components
    scene->setComponent(object, info.identifier, info.wall);

    glm::vec3 min = info.wall.position - glm::vec3(info.wall.width / 2.0f, info.wall.height / 2.0f, 0.0f);
    glm::vec3 max = info.wall.position + glm::vec3(info.wall.width / 2.0f, info.wall.height / 2.0f, 0.0f);

    Quad left = {
        .body = {
            .mass = std::numeric_limits<float>::infinity(),
        },
        .quad = {
            .width = info.wall.thickness,
            .height = info.wall.height,
        },
        .transform = {
            .position = {min.x - info.wall.thickness / 2.0f, info.wall.position.y, 0.0f},
        },
        .identifier = info.identifier,
        .hasPhysics = info.isCollidable,
    };

    Quad right = {
        .body = {
            .mass = std::numeric_limits<float>::infinity(),
        },
        .quad = {
            .width = info.wall.thickness,
            .height = info.wall.height,
        },
        .transform = {
            .position = {max.x + info.wall.thickness / 2.0f, info.wall.position.y, 0.0f},
        },
        .identifier = info.identifier,
        .hasPhysics = info.isCollidable,
    };

    Quad up = {
        .body = {
            .mass = std::numeric_limits<float>::infinity(),
        },
        .quad = {
            .width = info.wall.width,
            .height = info.wall.thickness,
        },
        .transform = {
            .position = {info.wall.position.x, min.y - info.wall.thickness / 2.0f, 0.0f},
        },
        .identifier = info.identifier,
        .hasPhysics = info.isCollidable,
    };

    Quad down = {
        .body = {
            .mass = std::numeric_limits<float>::infinity(),
        },
        .quad = {
            .width = info.wall.width,
            .height = info.wall.thickness,
        },
        .transform = {
            .position = {info.wall.position.x, max.y + info.wall.thickness / 2.0f, 0.0f},
        },
        .identifier = info.identifier,
        .hasPhysics = info.isCollidable,
    };

    // create a rebuild callback
    RebuildableComponent rebuilder = {
        .rebuild = [](ObjectBuilder &builder, Physbuzz::ObjectID object) {
            const WallComponent &wall = builder.scene->getComponent<WallComponent>(object);

            bool isCollidable = false;
            bool isRenderable = false;

            for (const auto &id : {wall.left, wall.right, wall.up, wall.down}) {
                isCollidable = builder.scene->containsComponent<Physbuzz::AABBComponent>(id) || isCollidable;
                isRenderable = builder.scene->containsComponent<Physbuzz::Mesh>(id) || isRenderable;

                builder.scene->eraseObject(id);
            }

            Wall info = {
                .wall = wall,
                .identifier = builder.scene->getComponent<IdentifiableComponent>(object),
                .isCollidable = isCollidable,
                .isRenderable = isRenderable,
            };

            builder.create(object, info);
        },
    };

    scene->setComponent(object, rebuilder);

    // TODO imposter syndrome has bested me
    // calling create within this scope nukes object's internal state for some obscure reason
    // ideally doing info.wall.left = create(left); and object.setComponent(info.wall); should be the better way
    WallComponent &wall = scene->getComponent<WallComponent>(object);
    wall.left = create(left);
    wall.right = create(right);
    wall.up = create(up);
    wall.down = create(down);

    return object;
}
