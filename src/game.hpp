#pragma once

#include "collision/collision.hpp"
#include "dynamics/dynamics.hpp"
#include "ui/handler.hpp"
#include "wall.hpp"
#include <physbuzz/clock.hpp>
#include <physbuzz/events.hpp>
#include <physbuzz/renderer.hpp>
#include <physbuzz/window.hpp>

class Game {
  public:
    Game();
    ~Game();

    void loop();

    inline static bool isRunning = false;

    // displaying and rendering
    inline static Physbuzz::Window window{};
    inline static Physbuzz::Renderer renderer{window};
    inline static Physbuzz::Clock clock;

    // ecs object tracking
    inline static Physbuzz::Scene scene{};

    // input handlers
    inline static Physbuzz::EventManager eventManager{window};

    // ImGui UI builder
    inline static InterfaceManager interface{renderer};

    // game physics
    inline static Dynamics dynamics;
    inline static Collision collision;
    inline static Wall wall{&scene};
};
