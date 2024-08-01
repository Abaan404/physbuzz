#pragma once

#include <physbuzz/events/window.hpp>
#include <physbuzz/window.hpp>
#include <unordered_set>

enum class BindingType {
    OneShot,
    Continous,
};

struct BindingInfo {
    std::function<void()> callback;
    BindingType type = BindingType::Continous;
};

class Bindings {
  public:
    Bindings(Physbuzz::Window *window);
    ~Bindings();

    void build();
    void destory();

    void tick();

    std::unordered_map<Physbuzz::Key, BindingInfo> keyboardCallbacks;
    std::unordered_map<Physbuzz::Mouse, BindingInfo> mouseButtonCallbacks;

  private:
    Physbuzz::Window *m_Window = nullptr;

    struct {
        Physbuzz::EventID key;
        Physbuzz::EventID mouse;
    } m_Events;

    // TODO handle modifiers?
    std::unordered_set<Physbuzz::Key> m_HeldKeys;
    std::unordered_set<Physbuzz::Mouse> m_HeldMouseButtons;
};
