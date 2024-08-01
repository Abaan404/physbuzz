#pragma once

#include <physbuzz/camera.hpp>

class Game;

class Player {
  public:
    Player(Game *game);
    ~Player();

    void build();
    void destroy();

    void resize(const glm::ivec2 &resolution);

    Physbuzz::Camera camera;

  private:
    Game *m_Game = nullptr;
    float m_Speed = 0.5f;
    float m_Sensitivity = 0.1f;
};
