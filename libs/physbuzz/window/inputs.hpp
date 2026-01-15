#pragma once

#include "../ecs/system.hpp"
#include "../events/handler.hpp"
#include "../events/window.hpp"

namespace Physbuzz {

class Window;

enum class CallbackType {
    OneShot,
    Continous,
};

struct InputEventComponent {
    struct Keyboard {
        Key key;
        CallbackType type = CallbackType::Continous;
        std::function<void(Scene &, const KeyEvent &)> callback;
    };

    struct MouseButton {
        Button button;
        CallbackType type = CallbackType::Continous;
        std::function<void(Scene &, const MouseButtonEvent &)> callback;
    };

    std::vector<Keyboard> keyboardCallbacks;
    std::vector<MouseButton> mouseCallbacks;
};

class InputEvents : public System<InputEventComponent> {
  public:
    InputEvents(const std::shared_ptr<Window> window);

    bool build() override;
    bool destroy() override;

    void tick();

  private:
    std::shared_ptr<Window> m_Window = nullptr;

    struct {
        EventID key;
        EventID mouse;
    } m_Events;

    std::unordered_map<Key, KeyEvent> m_HeldKeys;
    std::unordered_map<Button, MouseButtonEvent> m_HeldMouseButtons;
};

} // namespace Physbuzz
