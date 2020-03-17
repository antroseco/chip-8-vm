#include "catch.hpp"
#include "instruction.hpp"

TEST_CASE("Instruction struct exposes the raw opcode", "[instruction]")
{
    Instruction instruction = 0x1234;

    REQUIRE(instruction.raw == 0x1234);

    SECTION("Opcode exposed is two bytes long")
    {
        REQUIRE(sizeof(instruction.raw) == 2);
    }
}

TEST_CASE("Instruction struct can decode instructions", "[instruction]")
{
    Instruction instruction = 0x1234;

    SECTION("Decode x")
    {
        REQUIRE(instruction.x() == 0x2);
    }

    SECTION("Decode y")
    {
        REQUIRE(instruction.y() == 0x3);
    }

    SECTION("Decode n")
    {
        REQUIRE(instruction.n() == 0x4);
    }

    SECTION("Decode kk")
    {
        REQUIRE(instruction.kk() == 0x34);
    }

    SECTION("Decode nnn")
    {
        REQUIRE(instruction.nnn() == 0x234);
    }
}
