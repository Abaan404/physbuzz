#include "clock.hpp"

namespace Physbuzz {

Clock::Clock() {
    m_Init = std::chrono::steady_clock::now();
    m_Prev = m_Init;
}

Clock::~Clock() {}

std::uint32_t Clock::getTime() const {
    auto now = std::chrono::steady_clock::now();
    return std::chrono::duration_cast<std::chrono::milliseconds>(now - m_Init).count();
}

std::uint32_t Clock::getDelta() const {
    return std::chrono::duration_cast<std::chrono::milliseconds>(m_Delta).count();
}

void Clock::tick() {
    auto now = std::chrono::steady_clock::now();
    m_Delta = now - m_Prev;
    m_Prev = now;
}

} // namespace Physbuzz
