#include "wall.hpp"

#include "quad.hpp"
#include <glm/glm.hpp>
#include <physbuzz/renderer.hpp>

template <>
Physbuzz::ObjectID ObjectBuilder::create(Physbuzz::Object &object, Wall &info) {
    // user-defined components
    object.setComponent(info.transform);
    object.setComponent(info.identifier);
    object.setComponent(info.wall);

    glm::vec3 min = info.transform.position - glm::vec3(info.wall.width / 2.0f, info.wall.height / 2.0f, 0.0f);
    glm::vec3 max = info.transform.position + glm::vec3(info.wall.width / 2.0f, info.wall.height / 2.0f, 0.0f);

    Quad left = {
        .body = {
            .mass = std::numeric_limits<float>::infinity(),
        },
        .transform = {
            .position = {min.x - info.wall.thickness / 2.0f, info.transform.position.y, 0.0f},
        },
        .quad = {
            .width = info.wall.thickness,
            .height = info.wall.height,
        },
        .identifier = info.identifier,
        .isCollidable = info.isCollidable,
        .isRenderable = info.isRenderable,
    };

    Quad right = {
        .body = {
            .mass = std::numeric_limits<float>::infinity(),
        },
        .transform = {
            .position = {max.x + info.wall.thickness / 2.0f, info.transform.position.y, 0.0f},
        },
        .quad = {
            .width = info.wall.thickness,
            .height = info.wall.height,
        },
        .identifier = info.identifier,
        .isCollidable = info.isCollidable,
        .isRenderable = info.isRenderable,
    };

    Quad up = {
        .body = {
            .mass = std::numeric_limits<float>::infinity(),
        },
        .transform = {
            .position = {info.transform.position.x, min.y - info.wall.thickness / 2.0f, 0.0f},
        },
        .quad = {
            .width = info.wall.width,
            .height = info.wall.thickness,
        },
        .identifier = info.identifier,
        .isCollidable = info.isCollidable,
        .isRenderable = info.isRenderable,
    };

    Quad down = {
        .body = {
            .mass = std::numeric_limits<float>::infinity(),
        },
        .transform = {
            .position = {info.transform.position.x, max.y + info.wall.thickness / 2.0f, 0.0f},
        },
        .quad = {
            .width = info.wall.width,
            .height = info.wall.thickness,
        },
        .identifier = info.identifier,
        .isCollidable = info.isCollidable,
        .isRenderable = info.isRenderable,
    };

    // create a rebuild callback
    RebuildableComponent rebuilder = {
        .rebuild = [](ObjectBuilder &builder, Physbuzz::Object &object) {
            const WallComponent &wall = object.getComponent<WallComponent>();

            bool isCollidable;
            bool isRenderable;

            for (const auto &id : {wall.left, wall.right, wall.up, wall.down}) {
                Physbuzz::Object &side = builder.scene.getObject(id);
                if (side.hasComponent<Physbuzz::RenderComponent>()) {
                    side.getComponent<Physbuzz::RenderComponent>().destroy();
                }

                isCollidable = side.hasComponent<Physbuzz::AABBComponent>();
                isRenderable = side.hasComponent<Physbuzz::RenderComponent>();

                builder.scene.deleteObject(id);
            }

            Wall info = {
                .transform = object.getComponent<Physbuzz::TransformableComponent>(),
                .wall = wall,
                .identifier = object.getComponent<IdentifiableComponent>(),
                .isCollidable = isCollidable,
                .isRenderable = isRenderable,
            };

            object.eraseComponents();
            builder.create(object, info);
        },
    };

    object.setComponent(rebuilder);

    // TODO imposter syndrome has bested me
    // calling create within this scope nukes object's internal state for some obscure reason
    // ideally doing info.wall.left = create(left); and object.setComponent(info.wall); should be the better way
    WallComponent &wall = object.getComponent<WallComponent>();
    wall.left = create(left);
    wall.right = create(right);
    wall.up = create(up);
    wall.down = create(down);

    return object.getId();
}
