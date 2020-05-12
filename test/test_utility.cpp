#include "catch.hpp"
#include "utility.hpp"

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <numeric>
#include <vector>

TEST_CASE("byte_view initialization", "[byte_view]")
{
    SECTION("Can be default initialized")
    {
        byte_view bv;

        CHECK(bv.data() == nullptr);
        CHECK(bv.size() == 0);
        CHECK(bv.empty());
    }

    SECTION("Can be initialized from a pointer and a size")
    {
        std::array<std::uint8_t, 4> array;
        std::iota(array.begin(), array.end(), 0);

        byte_view bv{array.data(), array.size()};

        REQUIRE(bv.data() == array.data());
        REQUIRE(bv.size() == array.size());
        CHECK_FALSE(bv.empty());
        CHECK(std::equal(array.cbegin(), array.cend(), bv.data()));
    }

    SECTION("Can be initialized from std::vector")
    {
        std::vector<std::uint8_t> vector{4};
        std::iota(vector.begin(), vector.end(), 0);

        byte_view bv = vector;

        REQUIRE(bv.data() == vector.data());
        REQUIRE(bv.size() == vector.size());
        CHECK_FALSE(bv.empty());
        CHECK(std::equal(vector.cbegin(), vector.cend(), bv.data()));
    }
}

TEST_CASE("byte_view indexing", "[byte_view]")
{
    std::array<std::uint8_t, 8> array;
    std::iota(array.begin(), array.end(), 0);

    byte_view bv{array.data(), array.size()};

    REQUIRE(bv.data() == array.data());

    for (std::size_t i = 0; i < array.size(); ++i)
        REQUIRE(bv[i] == array[i]);
}
