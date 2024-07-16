#include "clock.hpp"

namespace Physbuzz {

Clock::Clock() {
    m_Init = std::chrono::steady_clock::now();
}

Clock::~Clock() {}

float Clock::getTime() const {
    auto now = std::chrono::steady_clock::now();
    std::chrono::microseconds duration = std::chrono::duration_cast<std::chrono::microseconds>(now - m_Init);

    return duration.count() / 1000.0f;
}

std::chrono::duration<float, std::milli> Clock::getDelta() const {
    return std::chrono::duration<float, std::milli>(m_Delta);
}

void Clock::tick() {
    auto now = std::chrono::steady_clock::now();
    m_Delta = now - m_Prev;
    m_Prev = now;
    m_Ticks++;
}

} // namespace Physbuzz
