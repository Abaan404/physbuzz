#include "clock.hpp"

namespace Physbuzz {

Clock::Clock() {
    m_Init = std::chrono::steady_clock::now();
}

Clock::Clock(const Clock &other) {
    if (this != &other) {
        copy(other);
    }
}

Clock Clock::operator=(const Clock &other) {
    if (this != &other) {
        copy(other);
    }

    return *this;
}

Clock::~Clock() {}

float Clock::getTime() const {
    auto now = std::chrono::steady_clock::now();
    std::chrono::microseconds duration = std::chrono::duration_cast<std::chrono::microseconds>(now - m_Init);

    return duration.count() / 1000.0f;
}

float Clock::getDelta() const {
    std::chrono::microseconds duration = std::chrono::duration_cast<std::chrono::microseconds>(m_Delta);

    return duration.count() / 1000.0f;
}

void Clock::copy(const Clock &other) {
    m_Init = other.m_Init;
    m_Prev = other.m_Prev;
    m_Delta = other.m_Delta;
    m_Ticks = other.m_Ticks;
}

void Clock::tick() {
    auto now = std::chrono::steady_clock::now();
    m_Delta = now - m_Prev;
    m_Prev = now;
    m_Ticks++;
}

} // namespace Physbuzz
