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

struct BindingComponent {
    struct Keyboard {
        Key key;
        CallbackType type = CallbackType::Continous;
        std::function<void(const KeyEvent &, Scene &)> callback;
    };

    struct MouseButton {
        Button button;
        CallbackType type = CallbackType::Continous;
        std::function<void(const MouseButtonEvent &, Scene &)> callback;
    };

    std::vector<Keyboard> keyboardCallbacks;
    std::vector<MouseButton> mouseCallbacks;
};

class Bindings : public System<BindingComponent> {
  public:
    Bindings(Window *window);

    bool build() override;
    bool destroy() override;

    void tick();

  private:
    Window *m_Window = nullptr;

    struct {
        EventID key;
        EventID mouse;
    } m_Events;

    std::unordered_map<Key, KeyEvent> m_HeldKeys;
    std::unordered_map<Button, MouseButtonEvent> m_HeldMouseButtons;
};

} // namespace Physbuzz
