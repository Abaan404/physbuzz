#pragma once

#include <physbuzz/events/window.hpp>
#include <physbuzz/window.hpp>

namespace Physbuzz {

enum class CallbackType {
    OneShot,
    Continous,
};

template <typename T>
concept BindingInputEventType =
    std::same_as<T, Physbuzz::KeyEvent> ||
    std::same_as<T, Physbuzz::MouseButtonEvent>;

template <typename T>
    requires BindingInputEventType<T>
struct BindingInfo {
    std::function<void(const T &)> callback;
    CallbackType type = CallbackType::Continous;
};

class Bindings {
  public:
    Bindings(Physbuzz::Window *window);
    ~Bindings();

    void build();
    void destory();

    void tick();

    std::unordered_map<Physbuzz::Key, BindingInfo<Physbuzz::KeyEvent>> keyboardCallbacks;
    std::unordered_map<Physbuzz::Mouse, BindingInfo<Physbuzz::MouseButtonEvent>> mouseButtonCallbacks;

  private:
    Physbuzz::Window *m_Window = nullptr;

    struct {
        Physbuzz::EventID key;
        Physbuzz::EventID mouse;
    } m_Events;

    std::unordered_map<Physbuzz::Key, Physbuzz::KeyEvent> m_HeldKeys;
    std::unordered_map<Physbuzz::Mouse, Physbuzz::MouseButtonEvent> m_HeldMouseButtons;
};

} // namespace Physbuzz
