#pragma once

#include "../events/window.hpp"
#include "../window/window.hpp"
#include "../ecs/system.hpp"

namespace Physbuzz {

enum class CallbackType {
    OneShot,
    Continous,
};

struct BindingComponent {
    struct Keyboard {
        Key key;
        CallbackType type = CallbackType::Continous;
        std::function<void(const KeyEvent &)> callback;
    };

    struct MouseButton {
        Button button;
        CallbackType type = CallbackType::Continous;
        std::function<void(const MouseButtonEvent &)> callback;
    };

    std::vector<Keyboard> keyboardCallbacks;
    std::vector<MouseButton> mouseCallbacks;
};

class Bindings : public System<BindingComponent> {
  public:
    Bindings(Window *window);

    void build();
    void destroy();

    void tick(Scene &scene);

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
