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
    SECTION("Normal operation")
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

    SECTION("Invalid address")
    {
        // Ensure I + j >= 0xfff
        const uint16_t i = GENERATE(take(10, random(0xff0, 0xfff)));
        const uint16_t j = GENERATE(take(10, random(0x0f, 0xff)));

        std::vector<uint16_t> instructions;
        instructions.push_back(0x6000 | j); // ld_kk (load j to V0)
        instructions.push_back(0xB000 | i); // jp_v0 (jump to i + V0)

        CPU cpu(make_rom(instructions), nullptr);

        REQUIRE_NOTHROW(cpu.Step());
        REQUIRE_THROWS_AS(cpu.Step(), std::out_of_range);
    }
}

TEST_CASE("call (2nnn)", "[cpu]")
{
    std::vector<uint16_t> instructions;
    for (std::uint16_t i = 0; i < 16; ++i)
        instructions.push_back(0x2200 | (i * 2 + 2)); // call (push PC to stack and jump to 0x200 + i)

    // Overflow the stack
    instructions.push_back(0x2000);

    CPU cpu(make_rom(instructions), nullptr);

    REQUIRE(cpu.read_pc() == 0x200);

    for (int i = 0; i < 16; ++i)
    {
        REQUIRE_NOTHROW(cpu.Step());
        REQUIRE(cpu.read_pc() == (0x200 | (i * 2 + 2)));
        REQUIRE(cpu.read_stack().top() == (0x200 | (i * 2)));
    }

    SECTION("Stack can only contain 16 values", "[cpu]")
    {
        REQUIRE_THROWS_AS(cpu.Step(), std::runtime_error);
    }
}

TEST_CASE("ret (00EE)", "[cpu]")
{
    SECTION("Normal operation")
    {
        std::vector<uint16_t> instructions;
        instructions.push_back(0x2202); // call (jump to next instruction)
        instructions.push_back(0x00EE); // ret (jump back to 0x200)

        CPU cpu(make_rom(instructions), nullptr);

        REQUIRE(cpu.read_pc() == 0x200);
        REQUIRE_NOTHROW(cpu.Step());
        REQUIRE(cpu.read_pc() == 0x202);
        REQUIRE(cpu.read_stack().top() == 0x200);

        REQUIRE_NOTHROW(cpu.Step());
        REQUIRE(cpu.read_pc() == 0x200);
        REQUIRE(cpu.read_stack().empty());
    }

    SECTION("Called with an empty stack")
    {
        std::vector<uint16_t> instructions;
        instructions.push_back(0x00EE); // ret (empty stack; should throw)

        CPU cpu(make_rom(instructions), nullptr);

        REQUIRE(cpu.read_stack().empty());
        REQUIRE_THROWS_AS(cpu.Step(), std::runtime_error);
    }
}

TEST_CASE("se_x_kk (3xkk)", "[cpu]")
{
    auto vx = GENERATE(range(0x0, 0xf + 1));
    auto kk = GENERATE(take(10, random(0x00, 0xff)));

    std::vector<uint16_t> instructions;
    instructions.push_back(0x6000 | (vx << 8) | kk);       // ld_kk (loads kk to register vx)
    instructions.push_back(0x3000 | (vx << 8) | kk);       // se_x_kk (skip next instruction)
    instructions.push_back(0);                             // noop (should be skipped)
    instructions.push_back(0x6000 | (vx << 8) | (kk ^ 1)); // ld_kk (loads something other than kk)
    instructions.push_back(0x3000 | (vx << 8) | kk);       // se_x_kk (should not skip next instruction)

    CPU cpu(make_rom(instructions), nullptr);

    REQUIRE_NOTHROW(cpu.Step());
    REQUIRE(cpu.read_pc() == 0x202);
    REQUIRE(cpu.read_registers()[vx] == kk);

    REQUIRE_NOTHROW(cpu.Step());
    REQUIRE(cpu.read_pc() == 0x206);

    REQUIRE_NOTHROW(cpu.Step());
    REQUIRE(cpu.read_pc() == 0x208);
    REQUIRE(cpu.read_registers()[vx] != kk);

    REQUIRE_NOTHROW(cpu.Step());
    REQUIRE(cpu.read_pc() == 0x20A);
}

TEST_CASE("se_x_y (5xy0)", "[cpu]")
{
    auto vx = GENERATE(range(0x0, 0xf + 1));
    auto vy = GENERATE(range(0x0, 0xf + 1));
    auto kk = GENERATE(take(10, random(0x00, 0xff)));

    std::vector<uint16_t> instructions;
    instructions.push_back(0x6000 | (vx << 8) | kk);        // ld_kk (loads kk to register vx)
    instructions.push_back(0x6000 | (vy << 8) | kk);        // ld_kk (loads kk to register vy)
    instructions.push_back(0x5000 | (vx << 8) | (vy << 4)); // se_x_y (skip next instruction)
    instructions.push_back(0);                              // noop (should be skipped)
    instructions.push_back(0x6000 | (vy << 8) | (kk ^ 1));  // ld_kk (loads something other than kk)
    instructions.push_back(0x5000 | (vx << 8) | (vy << 4)); // se_x_y (should not skip next instruction)

    CPU cpu(make_rom(instructions), nullptr);

    REQUIRE_NOTHROW(cpu.Step());
    REQUIRE(cpu.read_pc() == 0x202);
    REQUIRE(cpu.read_registers()[vx] == kk);

    REQUIRE_NOTHROW(cpu.Step());
    REQUIRE(cpu.read_pc() == 0x204);
    REQUIRE(cpu.read_registers()[vy] == kk);

    REQUIRE_NOTHROW(cpu.Step());
    REQUIRE(cpu.read_pc() == 0x208);

    REQUIRE_NOTHROW(cpu.Step());
    REQUIRE(cpu.read_pc() == 0x20A);
    REQUIRE(cpu.read_registers()[vy] != kk);

    REQUIRE_NOTHROW(cpu.Step());
    if (vx != vy)
        REQUIRE(cpu.read_pc() == 0x20C);
    else
        REQUIRE(cpu.read_pc() == 0x20E);
}

TEST_CASE("sne_x_kk (4xkk)", "[cpu]")
{
    auto vx = GENERATE(range(0x0, 0xf + 1));
    auto kk = GENERATE(take(10, random(0x00, 0xff)));

    std::vector<uint16_t> instructions;
    instructions.push_back(0x6000 | (vx << 8) | kk);       // ld_kk (loads kk to register vx)
    instructions.push_back(0x4000 | (vx << 8) | kk);       // sne_x_kk (should not skip next instruction)
    instructions.push_back(0x6000 | (vx << 8) | (kk ^ 1)); // ld_kk (loads something other than kk)
    instructions.push_back(0x4000 | (vx << 8) | kk);       // sne_x_kk (skip next instruction)

    CPU cpu(make_rom(instructions), nullptr);

    REQUIRE_NOTHROW(cpu.Step());
    REQUIRE(cpu.read_pc() == 0x202);
    REQUIRE(cpu.read_registers()[vx] == kk);

    REQUIRE_NOTHROW(cpu.Step());
    REQUIRE(cpu.read_pc() == 0x204);

    REQUIRE_NOTHROW(cpu.Step());
    REQUIRE(cpu.read_pc() == 0x206);
    REQUIRE(cpu.read_registers()[vx] != kk);

    REQUIRE_NOTHROW(cpu.Step());
    REQUIRE(cpu.read_pc() == 0x20A);
}

TEST_CASE("sne_x_y (9xy0)", "[cpu]")
{
    auto vx = GENERATE(range(0x0, 0xf + 1));
    auto vy = GENERATE(range(0x0, 0xf + 1));
    auto kk = GENERATE(take(10, random(0x00, 0xff)));

    std::vector<uint16_t> instructions;
    instructions.push_back(0x6000 | (vx << 8) | kk);        // ld_kk (loads kk to register vx)
    instructions.push_back(0x6000 | (vy << 8) | kk);        // ld_kk (loads kk to register vy)
    instructions.push_back(0x9000 | (vx << 8) | (vy << 4)); // sne_x_y (should not skip next instruction)
    instructions.push_back(0x6000 | (vy << 8) | (kk ^ 1));  // ld_kk (loads something other than kk)
    instructions.push_back(0x9000 | (vx << 8) | (vy << 4)); // sne_x_y (skip next instruction)

    CPU cpu(make_rom(instructions), nullptr);

    REQUIRE_NOTHROW(cpu.Step());
    REQUIRE(cpu.read_pc() == 0x202);
    REQUIRE(cpu.read_registers()[vx] == kk);

    REQUIRE_NOTHROW(cpu.Step());
    REQUIRE(cpu.read_pc() == 0x204);
    REQUIRE(cpu.read_registers()[vy] == kk);

    REQUIRE_NOTHROW(cpu.Step());
    REQUIRE(cpu.read_pc() == 0x206);

    REQUIRE_NOTHROW(cpu.Step());
    REQUIRE(cpu.read_pc() == 0x208);
    REQUIRE(cpu.read_registers()[vy] != kk);

    REQUIRE_NOTHROW(cpu.Step());
    if (vx != vy)
        REQUIRE(cpu.read_pc() == 0x20C);
    else
        REQUIRE(cpu.read_pc() == 0x20A);
}

TEST_CASE("ld_kk (6xkk)", "[cpu]")
{
    auto vx = GENERATE(range(0x0, 0xf + 1));
    auto kk = GENERATE(take(10, random(0x00, 0xff)));

    std::vector<uint16_t> instructions;
    instructions.push_back(0x6000 | (vx << 8) | kk); // ld_kk (loads kk into register vx)

    CPU cpu(make_rom(instructions), nullptr);

    REQUIRE_NOTHROW(cpu.Step());
    REQUIRE(cpu.read_registers()[vx] == kk);
}

TEST_CASE("ld_y (8xy0)", "[cpu]")
{
    auto vx = GENERATE(range(0x0, 0xf + 1));
    auto vy = GENERATE(range(0x0, 0xf + 1));
    auto kk = GENERATE(take(10, random(0x00, 0xff)));

    std::vector<uint16_t> instructions;
    instructions.push_back(0x6000 | (vy << 8) | kk);        // ld_kk (loads kk into register vy)
    instructions.push_back(0x8000 | (vx << 8) | (vy << 4)); // ld_y (loads vy into vx)

    CPU cpu(make_rom(instructions), nullptr);

    REQUIRE_NOTHROW(cpu.Step());
    REQUIRE(cpu.read_registers()[vy] == kk);

    REQUIRE_NOTHROW(cpu.Step());
    REQUIRE(cpu.read_registers()[vx] == kk);
}

TEST_CASE("ld_addr (Annn)", "[cpu]")
{
    auto address = GENERATE(take(100, random(0x000, 0xfff)));

    std::vector<uint16_t> instructions;
    instructions.push_back(0xA000 | address); // ld_addr (loads address into VI)

    CPU cpu(make_rom(instructions), nullptr);

    REQUIRE_NOTHROW(cpu.Step());
    REQUIRE(cpu.read_vi() == address);
}
