#include "collision/collision.hpp"
#include "dynamics/dynamics.hpp"
#include "ui/handler.hpp"
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

    inline static Physbuzz::Window window{};
    inline static Physbuzz::Scene scene{};
    inline static Physbuzz::Renderer renderer{window};
    inline static Physbuzz::EventManager eventManager{window};
    inline static Physbuzz::Clock clock;
    inline static InterfaceManager interface{renderer};
    inline static Dynamics dynamics;
    inline static Collision collision;
};
