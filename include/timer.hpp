#pragma once

#include <chrono>
#include <cstdint>
#include <type_traits>

class Timer
{
    typedef std::conditional<
        std::chrono::high_resolution_clock::is_steady,
        std::chrono::high_resolution_clock,
        std::chrono::steady_clock>::type clock_t;

    clock_t::time_point epoch;
    uint8_t value = 0;

public:
    Timer();
    ~Timer() = default;

    void set(uint8_t x) noexcept;
    uint8_t read() const noexcept;
};
