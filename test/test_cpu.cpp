#include "catch.hpp"
#include "cpu.hpp"

#include <arpa/inet.h>

std::vector<uint8_t> make_rom(const std::vector<uint16_t>& instructions)
{
    std::vector<uint8_t> rom;

    for (auto instruction : instructions)
    {
        instruction = htons(instruction);
        auto* const pointer = reinterpret_cast<uint8_t*>(&instruction);

        rom.push_back(pointer[0]);
        rom.push_back(pointer[1]);
    }

    return rom;
}

inline std::uint16_t operator"" _u(unsigned long long value)
{
    return static_cast<std::uint16_t>(value);
}

TEST_CASE("jp (1nnn)", "[cpu]")
{
    const uint16_t i = GENERATE(take(100, random(0x000, 0xfff)));
    std::vector<uint16_t> instructions;
    instructions.push_back(0x1000 | i);

    CPU cpu(make_rom(instructions), nullptr);

    REQUIRE(cpu.read_pc() == 0x200);

    cpu.Step();

    REQUIRE(cpu.read_pc() == i);
}
