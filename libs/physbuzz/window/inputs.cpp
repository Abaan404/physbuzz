#include "inputs.hpp"

#include "../ecs/scene.hpp"
#include "../window/window.hpp"
#include <tracy/Tracy.hpp>

namespace Physbuzz {

InputEvents::InputEvents(const std::shared_ptr<Window> window)
    : m_Window(window) {}

bool InputEvents::build() {
    if (!m_Window) {
        Logger::ERROR("[Binding] Building with a missing window");
        return false;
    }

    m_Events.key = m_Window->addCallback<KeyEvent>([&](const KeyEvent &event) {
        if (event.action == Action::Press) {
            m_HeldKeys[event.key] = event;
        } else if (event.action == Action::Release) {
            m_HeldKeys.erase(event.key);
        }
    });

    m_Events.mouse = m_Window->addCallback<MouseButtonEvent>([&](const MouseButtonEvent &event) {
        if (event.action == Action::Press) {
            m_HeldMouseButtons[event.button] = event;
        } else if (event.action == Action::Release) {
            m_HeldMouseButtons.erase(event.button);
        }
    });

    return true;
}

bool InputEvents::destroy() {
    if (m_Window == nullptr) {
        Logger::ERROR("[Inputs] Cant destroy with a missing window");
        return false;
    }

    m_Window->eraseCallback<KeyEvent>(m_Events.key);
    m_Window->eraseCallback<MouseButtonEvent>(m_Events.mouse);

    m_Window = nullptr;
    m_HeldMouseButtons.clear();
    m_HeldKeys.clear();

    return true;
}

void InputEvents::tick() {
    ZoneScopedN("InputEvents/Tick");

    if (m_Window == nullptr) {
        Logger::ERROR("[Inputs] Cant tick with a missing window");
        return;
    }

    m_Window->poll();

    for (const auto &object : m_Objects) {
        const auto [input] = m_Scene->getComponent<InputEventComponent>(object);

        for (const auto &input : input.keyboardCallbacks) {
            if (m_HeldKeys.contains(input.key)) {
                input.callback(*m_Scene, m_HeldKeys[input.key]);

                if (input.type == CallbackType::OneShot) {
                    m_HeldKeys.erase(input.key);
                }
            }
        }

        for (const auto &input : input.mouseCallbacks) {
            if (m_HeldMouseButtons.contains(input.button)) {
                input.callback(*m_Scene, m_HeldMouseButtons[input.button]);

                if (input.type == CallbackType::OneShot) {
                    m_HeldMouseButtons.erase(input.button);
                }
            }
        }
    }
}

} // namespace Physbuzz
