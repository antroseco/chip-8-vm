#include <atomic>
#include <future>
#include <thread>

class Timer
{
    std::atomic_uint8_t value = 0;
    std::thread worker;
    std::promise<void> stop_promise;
    std::future<void> stop_future = stop_promise.get_future();

    void decrement() noexcept;

public:
    Timer();
    ~Timer();

    void set(uint8_t x) noexcept;
    uint8_t read() const noexcept;
};
