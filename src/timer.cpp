#include "timer.hpp"

Timer::Timer() : epoch(clock_t::now()) {}

void Timer::set(uint8_t x) noexcept
{
    value = x;
    epoch = clock_t::now();
}

uint8_t Timer::read() const noexcept
{
    auto delta = clock_t::now() - epoch;
    auto ticks = delta / std::chrono::microseconds(16667);

    return ticks < value ? value - ticks : 0;
}
