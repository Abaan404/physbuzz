#pragma once

#include <chrono>

namespace Physbuzz {

class Clock {
  public:
    Clock();
    ~Clock();

    void tick();
    std::chrono::duration<float, std::milli> getDelta() const;
    float getTime() const;

  private:
    std::chrono::time_point<std::chrono::steady_clock> m_Init;
    std::chrono::time_point<std::chrono::steady_clock> m_Prev;
    std::chrono::steady_clock::duration m_Delta;

    std::uint32_t m_Ticks;
};

} // namespace Physbuzz
