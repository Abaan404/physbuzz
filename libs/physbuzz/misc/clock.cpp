#include "clock.hpp"

#include <tracy/Tracy.hpp>

namespace Physbuzz {

Clock::Clock() {
    m_Init = std::chrono::steady_clock::now();
    m_Prev = m_Init;
}

std::uint32_t Clock::getTime() const {
    auto now = std::chrono::steady_clock::now();
    return std::chrono::duration_cast<std::chrono::microseconds>(now - m_Init).count();
}

std::uint32_t Clock::getDelta() const {
    return std::chrono::duration_cast<std::chrono::microseconds>(m_Delta).count();
}

void Clock::tick() {
    ZoneScopedN("Clock/Tick");
    auto now = std::chrono::steady_clock::now();
    m_Delta = now - m_Prev;
    m_Prev = now;
}

} // namespace Physbuzz
