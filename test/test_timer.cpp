#include "catch.hpp"
#include "timer.hpp"

#include <algorithm>
#include <chrono>
#include <thread>

TEST_CASE("Timer value can be set and read", "[timer]")
{
    Timer timer;

    auto n = GENERATE(take(10, random(0x00, 0xff)));

    REQUIRE_NOTHROW(timer.set(n));
    REQUIRE(timer.read() == n);
}

TEST_CASE("Timer is decremented at a rate of 60 Hz", "[timer]")
{
    Timer timer;

    auto n = GENERATE(take(10, random(0x00, 0xff)));
    auto t = GENERATE(take(10, random(0, 200000)));

    REQUIRE_NOTHROW(timer.set(n));

    std::this_thread::sleep_for(std::chrono::microseconds(t));

    const auto expected = Approx(std::max(n - (t / 16667), 0)).margin(1);
    REQUIRE(timer.read() == expected);
}

TEST_CASE("Timer doesn't underflow")
{
    Timer timer;
    timer.set(1);

    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    REQUIRE(timer.read() == 0);
}
