#include "timer.hpp"

Timer::Timer() : worker(&Timer::decrement, this) {}

Timer::~Timer()
{
    if (worker.joinable())
    {
        stop_promise.set_value();
        worker.join();
    }
}

void Timer::decrement() noexcept // Clocks provided by the standard library never throw
{
    while (stop_future.wait_for(std::chrono::microseconds(16667)) == std::future_status::timeout)
    {
        uint8_t expected = value.load(std::memory_order_relaxed);
        uint8_t desired;
        do
        {
            if (expected == 0)
                break;

            desired = expected - 1;
        } while (!value.compare_exchange_weak(expected, desired, std::memory_order_relaxed));
    }
}

void Timer::set(uint8_t x) noexcept
{
    value.store(x, std::memory_order_relaxed);
}

uint8_t Timer::read() const noexcept
{
    return value.load(std::memory_order_relaxed);
}
