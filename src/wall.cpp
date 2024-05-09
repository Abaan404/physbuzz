#include "wall.hpp"

#include "objects/quad.hpp"
#include <glm/glm.hpp>

Wall::Wall(Physbuzz::Scene *scene) : m_Scene(scene) {
    m_Id = scene->createObject();
}

Wall::~Wall() {
    m_Scene->deleteObject(m_Id);
    if (m_Erected) {
        destroyWalls();
    }
}

bool Wall::isErect() const {
    return m_Erected;
}

void Wall::rebuild() {
    m_Erected = false;
    if (!m_Scene->hasObject(m_Id)) {
        m_Id = m_Scene->createObject();
        build(m_Info);
    }
}

void Wall::destroyWalls() {
    if (m_Info.isRenderable) {
        m_Scene->getObject(m_Left).getComponent<Physbuzz::MeshComponent>().destroy();
        m_Scene->getObject(m_Right).getComponent<Physbuzz::MeshComponent>().destroy();
        m_Scene->getObject(m_Up).getComponent<Physbuzz::MeshComponent>().destroy();
        m_Scene->getObject(m_Down).getComponent<Physbuzz::MeshComponent>().destroy();
    }

    m_Scene->deleteObject(m_Left);
    m_Scene->deleteObject(m_Right);
    m_Scene->deleteObject(m_Up);
    m_Scene->deleteObject(m_Down);
    m_Erected = false;
}

Physbuzz::ObjectID Wall::getId() const {
    return m_Id;
}

Physbuzz::ObjectID Wall::build(WallInfo &info) {
    Physbuzz::Object &object = m_Scene->getObject(m_Id);
    m_Info = info;

    // user-defined components
    object.setComponent(info.wall);
    object.setComponent(info.transform);
    object.setComponent(info.identifier);

    glm::vec3 min = info.transform.position - glm::vec3(info.wall.width / 2.0f, info.wall.height / 2.0f, 0.0f);
    glm::vec3 max = info.transform.position + glm::vec3(info.wall.width / 2.0f, info.wall.height / 2.0f, 0.0f);

    QuadInfo wallLeft = {
        .transform = {
            .position = {min.x, info.transform.position.y, 0.0f},
        },
        .body = {
            .mass = std::numeric_limits<float>::infinity(),
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
        .transform = {
            .position = {max.x, info.transform.position.y, 0.0f},
        },
        .body = {
            .mass = std::numeric_limits<float>::infinity(),
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
        .transform = {
            .position = {info.transform.position.x, min.y, 0.0f},
        },
        .body = {
            .mass = std::numeric_limits<float>::infinity(),
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
        .transform = {
            .position = {info.transform.position.x, max.y, 0.0f},
        },
        .body = {
            .mass = std::numeric_limits<float>::infinity(),
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
