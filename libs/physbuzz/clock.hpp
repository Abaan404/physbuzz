#pragma once

#include <chrono>
#include <ctime>

using namespace std::chrono;

namespace Physbuzz {

class Clock {
  public:
    Clock();
    Clock(const Clock &other);
    Clock &operator=(const Clock &other);
    ~Clock();

    void tick();
    float getDelta() const;
    float getTime() const;

  private:
    void copy(const Clock &other);

    std::chrono::time_point<std::chrono::steady_clock> m_Init;
    std::chrono::time_point<std::chrono::steady_clock> m_Prev;
    std::chrono::steady_clock::duration m_Delta;

    std::uint32_t m_Ticks;
};

} // namespace Physbuzz
