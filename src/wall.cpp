#include "wall.hpp"

#include "objects/quad.hpp"
#include <glm/glm.hpp>
#include <physbuzz/renderer.hpp>

Wall::Wall(Physbuzz::Scene *scene) : m_Scene(scene) {}

Wall::~Wall() {}

bool Wall::isErect() const {
    return m_Erected;
}

Physbuzz::ObjectID Wall::getId() const {
    return m_Id;
}

const WallInfo &Wall::getInfo() const {
    return m_Info;
}

Physbuzz::ObjectID Wall::build(const WallInfo &info) {
    if (!m_Scene->hasObject(m_Id)) {
        m_Id = m_Scene->createObject();
    }

    m_Info = info;
    Physbuzz::Object &object = m_Scene->getObject(m_Id);

    // user-defined components
    object.setComponent(m_Info.wall);
    object.setComponent(m_Info.transform);
    object.setComponent(m_Info.identifier);

    glm::vec3 min = info.transform.position - glm::vec3(info.wall.width / 2.0f, info.wall.height / 2.0f, 0.0f);
    glm::vec3 max = info.transform.position + glm::vec3(info.wall.width / 2.0f, info.wall.height / 2.0f, 0.0f);

    QuadInfo wallLeft = {
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
        .identifier = {
            .hidden = true,
        },
        .isCollidable = info.isCollidable,
        .isRenderable = info.isRenderable,
    };

    QuadInfo wallRight = {
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
        .identifier = {
            .hidden = true,
        },
        .isCollidable = info.isCollidable,
        .isRenderable = info.isRenderable,
    };

    QuadInfo wallUp = {
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
        .identifier = {
            .hidden = true,
        },
        .isCollidable = info.isCollidable,
        .isRenderable = info.isRenderable,
    };

    QuadInfo wallDown = {
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
        .identifier = {
            .hidden = true,
        },
        .isCollidable = info.isCollidable,
        .isRenderable = info.isRenderable,
    };

    if (m_Erected) {
        m_Left = ObjectBuilder<QuadInfo>::build(*m_Scene, m_Left, wallLeft);
        m_Right = ObjectBuilder<QuadInfo>::build(*m_Scene, m_Right, wallRight);
        m_Up = ObjectBuilder<QuadInfo>::build(*m_Scene, m_Up, wallUp);
        m_Down = ObjectBuilder<QuadInfo>::build(*m_Scene, m_Down, wallDown);

    } else {
        m_Left = ObjectBuilder<QuadInfo>::build(*m_Scene, wallLeft);
        m_Right = ObjectBuilder<QuadInfo>::build(*m_Scene, wallRight);
        m_Up = ObjectBuilder<QuadInfo>::build(*m_Scene, wallUp);
        m_Down = ObjectBuilder<QuadInfo>::build(*m_Scene, wallDown);

        m_Erected = true;
    }

    return m_Id;
}

void Wall::destroy() {
    if (m_Info.isRenderable) {
        m_Scene->getObject(m_Left).getComponent<Physbuzz::RenderComponent>().destroy();
        m_Scene->getObject(m_Right).getComponent<Physbuzz::RenderComponent>().destroy();
        m_Scene->getObject(m_Up).getComponent<Physbuzz::RenderComponent>().destroy();
        m_Scene->getObject(m_Down).getComponent<Physbuzz::RenderComponent>().destroy();
    }

    m_Scene->deleteObject(m_Left);
    m_Scene->deleteObject(m_Right);
    m_Scene->deleteObject(m_Up);
    m_Scene->deleteObject(m_Down);
    m_Scene->deleteObject(m_Id);
    m_Erected = false;
}
