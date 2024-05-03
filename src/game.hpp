#include "ui/handler.hpp"
#include <physbuzz/dynamics.hpp>
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
    inline static Physbuzz::Dynamics dynamics;
    inline static InterfaceManager interface{renderer};
};
