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

#define TEST_GOOD_OPCODE(x, expected)              \
    SECTION(#x)                                    \
    {                                              \
        Instruction instruction = x;               \
        REQUIRE(instruction.opcode() == expected); \
    }

#define TEST_BAD_OPCODE(x)                    \
    SECTION(#x)                               \
    {                                         \
        Instruction instruction = x;          \
        REQUIRE_THROWS(instruction.opcode()); \
    }

TEST_CASE("Instruction struct can decode the correct opcode", "[instruction]")
{
    // 0x0000 - 0x4000
    TEST_GOOD_OPCODE(0x0123, 0x0000)
    TEST_GOOD_OPCODE(0x00E0, 0x00E0);
    TEST_GOOD_OPCODE(0x00EE, 0x00EE);
    TEST_GOOD_OPCODE(0x1123, 0x1000);
    TEST_GOOD_OPCODE(0x2123, 0x2000);
    TEST_GOOD_OPCODE(0x3123, 0x3000);
    TEST_GOOD_OPCODE(0x4123, 0x4000);

    // 0x5000
    TEST_GOOD_OPCODE(0x5120, 0x5000);
    TEST_BAD_OPCODE(0x5121);
    TEST_BAD_OPCODE(0x5122);
    TEST_BAD_OPCODE(0x5123);
    TEST_BAD_OPCODE(0x5124);
    TEST_BAD_OPCODE(0x5125);
    TEST_BAD_OPCODE(0x5126);
    TEST_BAD_OPCODE(0x5127);
    TEST_BAD_OPCODE(0x5128);
    TEST_BAD_OPCODE(0x5129);
    TEST_BAD_OPCODE(0x512A);
    TEST_BAD_OPCODE(0x512B);
    TEST_BAD_OPCODE(0x512C);
    TEST_BAD_OPCODE(0x512D);
    TEST_BAD_OPCODE(0x512E);
    TEST_BAD_OPCODE(0x512F);

    // 0x6000 - 0x7000
    TEST_GOOD_OPCODE(0x6213, 0x6000);
    TEST_GOOD_OPCODE(0x7123, 0x7000);

    // 0x8000
    TEST_GOOD_OPCODE(0x8120, 0x8000);
    TEST_GOOD_OPCODE(0x8121, 0x8001);
    TEST_GOOD_OPCODE(0x8122, 0x8002);
    TEST_GOOD_OPCODE(0x8123, 0x8003);
    TEST_GOOD_OPCODE(0x8124, 0x8004);
    TEST_GOOD_OPCODE(0x8125, 0x8005);
    TEST_GOOD_OPCODE(0x8126, 0x8006);
    TEST_GOOD_OPCODE(0x8127, 0x8007);
    TEST_BAD_OPCODE(0x8128);
    TEST_BAD_OPCODE(0x8129);
    TEST_BAD_OPCODE(0x812A);
    TEST_BAD_OPCODE(0x812B);
    TEST_BAD_OPCODE(0x812C);
    TEST_BAD_OPCODE(0x812D);
    TEST_GOOD_OPCODE(0x812E, 0x800E);
    TEST_BAD_OPCODE(0x812F);

    // 0x9000
    TEST_GOOD_OPCODE(0x9120, 0x9000);
    TEST_BAD_OPCODE(0x9121);
    TEST_BAD_OPCODE(0x9122);
    TEST_BAD_OPCODE(0x9123);
    TEST_BAD_OPCODE(0x9124);
    TEST_BAD_OPCODE(0x9125);
    TEST_BAD_OPCODE(0x9126);
    TEST_BAD_OPCODE(0x9127);
    TEST_BAD_OPCODE(0x9128);
    TEST_BAD_OPCODE(0x9129);
    TEST_BAD_OPCODE(0x912A);
    TEST_BAD_OPCODE(0x912B);
    TEST_BAD_OPCODE(0x912C);
    TEST_BAD_OPCODE(0x912D);
    TEST_BAD_OPCODE(0x912E);
    TEST_BAD_OPCODE(0x912F);

    // 0xA000 - 0xD000
    TEST_GOOD_OPCODE(0xA123, 0xA000)
    TEST_GOOD_OPCODE(0xB123, 0xB000);
    TEST_GOOD_OPCODE(0xC123, 0xC000);
    TEST_GOOD_OPCODE(0xD123, 0xD000);

    // 0xE000
    TEST_GOOD_OPCODE(0xE19E, 0xE09E);
    TEST_GOOD_OPCODE(0xE1A1, 0xE0A1);
    TEST_BAD_OPCODE(0xE123)

    // 0xF000
    TEST_GOOD_OPCODE(0xF107, 0xF007);
    TEST_GOOD_OPCODE(0xF10A, 0xF00A);
    TEST_GOOD_OPCODE(0xF115, 0xF015);
    TEST_GOOD_OPCODE(0xF118, 0xF018);
    TEST_GOOD_OPCODE(0xF11E, 0xF01E);
    TEST_GOOD_OPCODE(0xF129, 0xF029);
    TEST_GOOD_OPCODE(0xF133, 0xF033);
    TEST_GOOD_OPCODE(0xF155, 0xF055);
    TEST_GOOD_OPCODE(0xF165, 0xF065);
    TEST_BAD_OPCODE(0xF123);
}
