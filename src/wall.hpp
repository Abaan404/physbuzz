#pragma once

#include "dynamics/dynamics.hpp"
#include "objects/objects.hpp"

struct WallComponent {
    float width = 0.0f;
    float height = 0.0f;
    float thickness = 0.0f;
};

struct WallInfo {
    // physics info
    TransformableComponent transform;

    // geometry
    WallComponent wall;

    // naming
    IdentifiableComponent identifier = {
        .hidden = true,
    };

    bool isCollidable = false;
    bool isRenderable = false;
};

class Wall {
  public:
    Wall(Physbuzz::Scene *scene);
    ~Wall();

    bool isErect() const;
    Physbuzz::ObjectID getId() const;
    const WallInfo &getInfo() const;

    Physbuzz::ObjectID build(const WallInfo &info);
    void destroy();

  private:
    Physbuzz::ObjectID m_Left = 0;
    Physbuzz::ObjectID m_Right = 0;
    Physbuzz::ObjectID m_Up = 0;
    Physbuzz::ObjectID m_Down = 0;

    Physbuzz::ObjectID m_Id = 0;

    WallInfo m_Info;
    Physbuzz::Scene *m_Scene;
    bool m_Erected = false;
};
