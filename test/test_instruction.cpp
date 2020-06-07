#include "catch.hpp"
#include "instruction.hpp"

#include <array>
#include <cstdint>

TEST_CASE("Instruction struct can be initialized with an address or value", "[instruction]")
{
    Instruction instruction;
    std::array<std::uint8_t, 2> buffer{0x12, 0x34};

    SECTION("Can be default initialized")
    {
        REQUIRE(instruction.raw == 0);
    }

    SECTION("Can read an address")
    {
        instruction.read(buffer.data());

        REQUIRE(instruction.raw == 0x1234);
    }

    SECTION("Can be initialized with a memory address")
    {
        Instruction instruction2{buffer.data()};

        REQUIRE(instruction2.raw == 0x1234);
    }
}

TEST_CASE("Instruction struct exposes the raw opcode", "[instruction]")
{
    const std::uint16_t i = GENERATE(take(100, random(0x0000, 0xffff)));
    const Instruction instruction{i};

    REQUIRE(instruction.raw == i);
}

TEST_CASE("Instruction struct can decode instructions", "[instruction]")
{
    const std::uint16_t i = GENERATE(take(100, random(0x0000, 0xffff)));
    const Instruction instruction{i};

    SECTION("Decode group")
    {
        REQUIRE(instruction.group() == ((instruction.raw & 0xF000) >> 12));
    }

    SECTION("Decode x")
    {
        REQUIRE(instruction.x() == ((instruction.raw & 0x0F00) >> 8));
    }

    SECTION("Decode y")
    {
        REQUIRE(instruction.y() == ((instruction.raw & 0x00F0) >> 4));
    }

    SECTION("Decode n")
    {
        REQUIRE(instruction.n() == (instruction.raw & 0x000F));
    }

    SECTION("Decode kk")
    {
        REQUIRE(instruction.kk() == (instruction.raw & 0x00FF));
    }

    SECTION("Decode nnn")
    {
        REQUIRE(instruction.nnn() == (instruction.raw & 0x0FFF));
    }
}
