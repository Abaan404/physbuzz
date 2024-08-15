#include "bindings.hpp"

#include "../debug/logging.hpp"

namespace Physbuzz {

Bindings::Bindings(Physbuzz::Window *window)
    : m_Window(window) {}

Bindings::~Bindings() {}

void Bindings::build() {
    if (!m_Window) {
        Physbuzz::Logger::ERROR("[Binding] Building with a missing window");
        return;
    }

    m_Events.key = m_Window->addCallback<Physbuzz::KeyEvent>([&](const Physbuzz::KeyEvent &event) {
        if (event.action == Physbuzz::Action::Press) {
            m_HeldKeys[event.key] = event;
        } else if (event.action == Physbuzz::Action::Release) {
            m_HeldKeys.erase(event.key);
        }
    });

    m_Events.mouse = m_Window->addCallback<Physbuzz::MouseButtonEvent>([&](const Physbuzz::MouseButtonEvent &event) {
        if (event.action == Physbuzz::Action::Press) {
            m_HeldMouseButtons[event.button] = event;
        } else if (event.action == Physbuzz::Action::Release) {
            m_HeldMouseButtons.erase(event.button);
        }
    });
}

void Bindings::destory() {
    if (m_Window == nullptr) {
        Physbuzz::Logger::ERROR("[Binding] Destroying with a missing window");
        return;
    }

    m_Window->eraseCallback<Physbuzz::KeyEvent>(m_Events.key);
    m_Window->eraseCallback<Physbuzz::KeyEvent>(m_Events.mouse);

    keyboardCallbacks.clear();
    mouseButtonCallbacks.clear();
    m_HeldMouseButtons.clear();
    m_HeldKeys.clear();
}

void Bindings::poll() {
    if (m_Window == nullptr) {
        Physbuzz::Logger::ERROR("[Binding] Ticking with a missing window");
        return;
    }

    m_Window->poll();

    for (const auto &[key, info] : keyboardCallbacks) {
        if (m_HeldKeys.contains(key)) {
            info.callback(m_HeldKeys[key]);

            if (info.type == CallbackType::OneShot) {
                m_HeldKeys.erase(key);
            }
        }
    }

    for (const auto &[button, info] : mouseButtonCallbacks) {
        if (m_HeldMouseButtons.contains(button)) {
            info.callback(m_HeldMouseButtons[button]);

            if (info.type == CallbackType::OneShot) {
                m_HeldMouseButtons.erase(button);
            }
        }
    }
}

} // namespace Physbuzz
