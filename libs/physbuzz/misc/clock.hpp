#pragma once

#include <chrono>

namespace Physbuzz {

class Clock {
  public:
    Clock();
    ~Clock();

    void tick();
    std::uint32_t getDelta() const;
    std::uint32_t getTime() const;

  private:
    std::chrono::time_point<std::chrono::steady_clock> m_Init;
    std::chrono::time_point<std::chrono::steady_clock> m_Prev;
    std::chrono::steady_clock::duration m_Delta;
};

} // namespace Physbuzz
