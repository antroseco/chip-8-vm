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

TEST_CASE("jp_v0 (Bnnn)", "[cpu]")
{
    // Ensure i + j < 0xfff
    const uint16_t i = GENERATE(take(10, random(0x000, 0xf00)));
    const uint16_t j = GENERATE(take(10, random(0x00, 0xff)));

    std::vector<uint16_t> instructions;
    instructions.push_back(0x6000 | j); // ld_kk (load j to V0)
    instructions.push_back(0xB000 | i); // jp_v0 (jump to i + V0)

    CPU cpu(make_rom(instructions), nullptr);

    REQUIRE(cpu.read_pc() == 0x200);

    REQUIRE_NOTHROW(cpu.Step());
    REQUIRE(cpu.read_registers()[0] == j);

    REQUIRE_NOTHROW(cpu.Step());
    REQUIRE(cpu.read_pc() == i + j);
}
