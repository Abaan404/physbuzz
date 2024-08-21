#pragma once

#include <physbuzz/render/camera.hpp>

class Game;

class Player {
  public:
    Player(Game *game);
    ~Player();

    void build();
    void destroy();

    void resize(const glm::ivec2 &resolution);

    Physbuzz::Camera camera;

    float speed = 0.5f;
    float sensitivity = 0.1f;

  private:
    bool m_CaptureMouse = false;
    Game *m_Game = nullptr;
};
