#include "bindings.hpp"
#include "physbuzz/logging.hpp"

Bindings::Bindings(Physbuzz::Window *window)
    : m_Window(window) {}

Bindings::~Bindings() {}

void Bindings::build() {
    if (m_Window == nullptr) {
        return;
    }

    m_Events.key = m_Window->addCallback<Physbuzz::KeyEvent>([&](const Physbuzz::KeyEvent &event) {
        if (event.action == Physbuzz::Action::Press) {
            m_HeldKeys.insert(event.key);
        } else if (event.action == Physbuzz::Action::Release) {
            m_HeldKeys.erase(event.key);
        }
    });

    m_Events.mouse = m_Window->addCallback<Physbuzz::MouseButtonEvent>([&](const Physbuzz::MouseButtonEvent &event) {
        if (event.action == Physbuzz::Action::Press) {
            m_HeldMouseButtons.insert(event.button);
        } else if (event.action == Physbuzz::Action::Release) {
            m_HeldMouseButtons.erase(event.button);
        }
    });
}

void Bindings::destory() {
    if (m_Window == nullptr) {
        return;
    }

    m_Window->removeCallback<Physbuzz::KeyEvent>(m_Events.key);
    m_Window->removeCallback<Physbuzz::KeyEvent>(m_Events.mouse);

    keyboardCallbacks.clear();
    mouseButtonCallbacks.clear();
    m_HeldMouseButtons.clear();
    m_HeldKeys.clear();
}

void Bindings::tick() {
    Physbuzz::Logger::ASSERT(m_Window != nullptr, "[BINDING] Window does not exist");

    if (m_Window == nullptr) {
        return;
    }

    for (const auto &[key, info] : keyboardCallbacks) {
        if (m_HeldKeys.contains(key)) {
            info.callback();

            if (info.type == BindingType::OneShot) {
                m_HeldKeys.erase(key);
            }
        }
    }

    for (const auto &[key, info] : mouseButtonCallbacks) {
        if (m_HeldMouseButtons.contains(key)) {
            info.callback();

            if (info.type == BindingType::OneShot) {
                m_HeldMouseButtons.erase(key);
            }
        }
    }
}
