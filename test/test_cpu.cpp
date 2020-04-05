#include "catch.hpp"
#include "cpu.hpp"

#include <arpa/inet.h>
#include <array>
#include <vector>

namespace
{

std::vector<std::uint8_t> make_rom(const std::uint16_t* begin, const std::uint16_t* end)
{
    std::vector<std::uint8_t> rom;
    rom.reserve(std::distance(begin, end) * 2);

    for (; begin != end; ++begin)
    {
        const std::uint16_t instruction = htons(*begin);
        const std::uint8_t* const pointer = reinterpret_cast<const std::uint8_t*>(&instruction);

        rom.emplace_back(pointer[0]);
        rom.emplace_back(pointer[1]);
    }

    return rom;
}

template <typename T>
auto range_i(T start, T end)
{
    // Returns the range from start to end _inclusive_
    return Catch::Generators::range(start, end + 1);
}

// Prevents integer promotion of result to int
[[nodiscard]] constexpr inline std::uint16_t b_or(std::uint16_t a, std::uint16_t b)
{
    return a | b;
}

[[nodiscard]] constexpr inline std::uint16_t b_or(std::uint16_t a, std::uint16_t b, std::uint16_t c)
{
    return a | b | c;
}

} // namespace

TEST_CASE("jp (1nnn)", "[cpu]")
{
    SECTION("Normal operation")
    {
        const std::uint16_t i = GENERATE(take(100, random(0x000, 0xffe)));
        const std::array<std::uint16_t, 1> instructions{
            b_or(0x1000, i) // jp (jump to i)
        };

        CPU cpu(make_rom(instructions.cbegin(), instructions.cend()), nullptr);

        REQUIRE(cpu.read_pc() == 0x200);

        REQUIRE_NOTHROW(cpu.step());

        REQUIRE(cpu.read_pc() == i);
    }

    SECTION("Address is the last byte (can't contain a 16 bit instruction)")
    {
        constexpr std::array<std::uint16_t, 1> instructions{
            0x1fff // jp (jump to 0xfff
        };

        CPU cpu(make_rom(instructions.cbegin(), instructions.cend()), nullptr);

        REQUIRE_THROWS_AS(cpu.step(), std::out_of_range);
    }

    SECTION("Jumping to the current instruction doesn't update the PC")
    {
        constexpr std::array<std::uint16_t, 1> instructions{
            0x1200 // jp (jump to 0x200)
        };

        CPU cpu(make_rom(instructions.cbegin(), instructions.cend()), nullptr);

        REQUIRE(cpu.read_pc() == 0x200);

        REQUIRE_NOTHROW(cpu.step());

        REQUIRE(cpu.read_pc() == 0x200);
    }
}

TEST_CASE("jp_v0 (Bnnn)", "[cpu]")
{
    SECTION("Normal operation")
    {
        // Ensure i + j < 0xfff
        const std::uint16_t i = GENERATE(take(10, random(0x000, 0xf00)));
        const std::uint16_t j = GENERATE(take(10, random(0x00, 0xff)));

        const std::array<std::uint16_t, 2> instructions{
            b_or(0x6000, j), // ld_kk (load j to V0)
            b_or(0xB000, i)  // jp_v0 (jump to i + V0)
        };

        CPU cpu(make_rom(instructions.cbegin(), instructions.cend()), nullptr);

        REQUIRE(cpu.read_pc() == 0x200);

        REQUIRE_NOTHROW(cpu.step());
        REQUIRE(cpu.read_registers()[0] == j);

        REQUIRE_NOTHROW(cpu.step());
        REQUIRE(cpu.read_pc() == i + j);
    }

    SECTION("Address out of range")
    {
        // Ensure I + j >= 0xfff
        const std::uint16_t i = GENERATE(take(10, random(0xff0, 0xfff)));
        const std::uint16_t j = GENERATE(take(10, random(0x0f, 0xff)));

        const std::array<std::uint16_t, 2> instructions{
            b_or(0x6000, j), // ld_kk (load j to V0)
            b_or(0xB000, i)  // jp_v0 (jump to i + V0)
        };

        CPU cpu(make_rom(instructions.cbegin(), instructions.cend()), nullptr);

        REQUIRE_NOTHROW(cpu.step());
        REQUIRE_THROWS_AS(cpu.step(), std::out_of_range);
    }

    SECTION("Jumping to the current instruction doesn't update the PC")
    {
        constexpr std::array<std::uint16_t, 2> instructions{
            0x60ff, // ld_kk (load 0xff to V0)
            0xB103  // jp_v0 (jump to 0x103 + V0)
        };

        CPU cpu(make_rom(instructions.cbegin(), instructions.cend()), nullptr);

        REQUIRE_NOTHROW(cpu.step());
        REQUIRE(cpu.read_registers()[0] == 0xff);
        REQUIRE(cpu.read_pc() == 0x202);

        REQUIRE_NOTHROW(cpu.step());
        REQUIRE(cpu.read_pc() == 0x202);
    }
}

TEST_CASE("call (2nnn)", "[cpu]")
{
    std::array<std::uint16_t, 17> instructions;
    for (std::uint16_t i = 0; i < 16; ++i)
        instructions[i] = 0x2200 | (i * 2 + 2); // call (push PC to stack and jump to 0x200 + i)

    // Overflow the stack
    instructions.back() = 0x2000;

    CPU cpu(make_rom(instructions.cbegin(), instructions.cend()), nullptr);

    REQUIRE(cpu.read_pc() == 0x200);

    for (int i = 0; i < 16; ++i)
    {
        REQUIRE_NOTHROW(cpu.step());
        REQUIRE(cpu.read_pc() == (0x200 | (i * 2 + 2)));
        REQUIRE(cpu.read_stack().top() == (0x200 | (i * 2)));
    }

    SECTION("Stack can only contain 16 values", "[cpu]")
    {
        REQUIRE_THROWS_AS(cpu.step(), std::runtime_error);
    }
}

TEST_CASE("ret (00EE)", "[cpu]")
{
    SECTION("Normal operation")
    {
        constexpr std::array<std::uint16_t, 2> instructions{
            0x2202, // call (jump to next instruction)
            0x00EE  // ret (jump back to 0x200)
        };

        CPU cpu(make_rom(instructions.cbegin(), instructions.cend()), nullptr);

        REQUIRE(cpu.read_pc() == 0x200);
        REQUIRE_NOTHROW(cpu.step());
        REQUIRE(cpu.read_pc() == 0x202);
        REQUIRE(cpu.read_stack().top() == 0x200);

        REQUIRE_NOTHROW(cpu.step());
        // PC should be equal to the value on the stack + 2
        REQUIRE(cpu.read_pc() == 0x202);
        REQUIRE(cpu.read_stack().empty());
    }

    SECTION("Called with an empty stack")
    {
        constexpr std::array<std::uint16_t, 1> instructions{
            0x00EE // ret (empty stack; should throw)
        };

        CPU cpu(make_rom(instructions.cbegin(), instructions.cend()), nullptr);

        REQUIRE(cpu.read_stack().empty());
        REQUIRE_THROWS_AS(cpu.step(), std::runtime_error);
    }
}

TEST_CASE("se_x_kk (3xkk)", "[cpu]")
{
    auto vx = GENERATE(range_i(0x0, 0xf));
    auto kk = GENERATE(take(10, random(0x00, 0xff)));

    const std::array<std::uint16_t, 5> instructions{
        b_or(0x6000, vx << 8, kk),     // ld_kk (loads kk to register vx)
        b_or(0x3000, vx << 8, kk),     // se_x_kk (skip next instruction)
        0,                             // noop (should be skipped)
        b_or(0x6000, vx << 8, kk ^ 1), // ld_kk (loads something other than kk)
        b_or(0x3000, vx << 8, kk)      // se_x_kk (should not skip next instruction)
    };

    CPU cpu(make_rom(instructions.cbegin(), instructions.cend()), nullptr);

    REQUIRE_NOTHROW(cpu.step());
    REQUIRE(cpu.read_pc() == 0x202);
    REQUIRE(cpu.read_registers()[vx] == kk);

    REQUIRE_NOTHROW(cpu.step());
    REQUIRE(cpu.read_pc() == 0x206);

    REQUIRE_NOTHROW(cpu.step());
    REQUIRE(cpu.read_pc() == 0x208);
    REQUIRE(cpu.read_registers()[vx] != kk);

    REQUIRE_NOTHROW(cpu.step());
    REQUIRE(cpu.read_pc() == 0x20A);
}

TEST_CASE("se_x_y (5xy0)", "[cpu]")
{
    auto vx = GENERATE(range_i(0x0, 0xf));
    auto vy = GENERATE(range_i(0x0, 0xf));
    auto kk = GENERATE(take(10, random(0x00, 0xff)));

    const std::array<std::uint16_t, 6> instructions{
        b_or(0x6000, vx << 8, kk),      // ld_kk (loads kk to register vx)
        b_or(0x6000, vy << 8, kk),      // ld_kk (loads kk to register vy)
        b_or(0x5000, vx << 8, vy << 4), // se_x_y (skip next instruction)
        0,                              // noop(should be skipped)
        b_or(0x6000, vy << 8, kk ^ 1),  // ld_kk (loads something other than kk)
        b_or(0x5000, vx << 8, vy << 4)  // se_x_y (should not skip next instruction)
    };

    CPU cpu(make_rom(instructions.cbegin(), instructions.cend()), nullptr);

    REQUIRE_NOTHROW(cpu.step());
    REQUIRE(cpu.read_pc() == 0x202);
    REQUIRE(cpu.read_registers()[vx] == kk);

    REQUIRE_NOTHROW(cpu.step());
    REQUIRE(cpu.read_pc() == 0x204);
    REQUIRE(cpu.read_registers()[vy] == kk);

    REQUIRE_NOTHROW(cpu.step());
    REQUIRE(cpu.read_pc() == 0x208);

    REQUIRE_NOTHROW(cpu.step());
    REQUIRE(cpu.read_pc() == 0x20A);
    REQUIRE(cpu.read_registers()[vy] != kk);

    REQUIRE_NOTHROW(cpu.step());
    if (vx != vy)
        REQUIRE(cpu.read_pc() == 0x20C);
    else
        REQUIRE(cpu.read_pc() == 0x20E);
}

TEST_CASE("sne_x_kk (4xkk)", "[cpu]")
{
    auto vx = GENERATE(range_i(0x0, 0xf));
    auto kk = GENERATE(take(10, random(0x00, 0xff)));

    const std::array<std::uint16_t, 4> instructions{
        b_or(0x6000, vx << 8, kk),     // ld_kk (loads kk to register vx)
        b_or(0x4000, vx << 8, kk),     // sne_x_kk (should not skip next instruction)
        b_or(0x6000, vx << 8, kk ^ 1), // ld_kk (loads something other than kk)
        b_or(0x4000, vx << 8, kk)      // sne_x_kk (skip next instruction)
    };

    CPU cpu(make_rom(instructions.cbegin(), instructions.cend()), nullptr);

    REQUIRE_NOTHROW(cpu.step());
    REQUIRE(cpu.read_pc() == 0x202);
    REQUIRE(cpu.read_registers()[vx] == kk);

    REQUIRE_NOTHROW(cpu.step());
    REQUIRE(cpu.read_pc() == 0x204);

    REQUIRE_NOTHROW(cpu.step());
    REQUIRE(cpu.read_pc() == 0x206);
    REQUIRE(cpu.read_registers()[vx] != kk);

    REQUIRE_NOTHROW(cpu.step());
    REQUIRE(cpu.read_pc() == 0x20A);
}

TEST_CASE("sne_x_y (9xy0)", "[cpu]")
{
    auto vx = GENERATE(range_i(0x0, 0xf));
    auto vy = GENERATE(range_i(0x0, 0xf));
    auto kk = GENERATE(take(10, random(0x00, 0xff)));

    const std::array<std::uint16_t, 5> instructions{
        b_or(0x6000, vx << 8, kk),      // ld_kk (loads kk to register vx)
        b_or(0x6000, vy << 8, kk),      // ld_kk (loads kk to register vy)
        b_or(0x9000, vx << 8, vy << 4), // sne_x_y (should not skip next instruction)
        b_or(0x6000, vy << 8, kk ^ 1),  // ld_kk (loads something other than kk)
        b_or(0x9000, vx << 8, vy << 4)  // sne_x_y (skip next instruction)
    };

    CPU cpu(make_rom(instructions.cbegin(), instructions.cend()), nullptr);

    REQUIRE_NOTHROW(cpu.step());
    REQUIRE(cpu.read_pc() == 0x202);
    REQUIRE(cpu.read_registers()[vx] == kk);

    REQUIRE_NOTHROW(cpu.step());
    REQUIRE(cpu.read_pc() == 0x204);
    REQUIRE(cpu.read_registers()[vy] == kk);

    REQUIRE_NOTHROW(cpu.step());
    REQUIRE(cpu.read_pc() == 0x206);

    REQUIRE_NOTHROW(cpu.step());
    REQUIRE(cpu.read_pc() == 0x208);
    REQUIRE(cpu.read_registers()[vy] != kk);

    REQUIRE_NOTHROW(cpu.step());
    if (vx != vy)
        REQUIRE(cpu.read_pc() == 0x20C);
    else
        REQUIRE(cpu.read_pc() == 0x20A);
}

TEST_CASE("ld_kk (6xkk)", "[cpu]")
{
    auto vx = GENERATE(range_i(0x0, 0xf));
    auto kk = GENERATE(take(10, random(0x00, 0xff)));

    const std::array<std::uint16_t, 1> instructions{
        b_or(0x6000, vx << 8, kk) // ld_kk (loads kk into register vx)
    };

    CPU cpu(make_rom(instructions.cbegin(), instructions.cend()), nullptr);

    REQUIRE_NOTHROW(cpu.step());
    REQUIRE(cpu.read_registers()[vx] == kk);
}

TEST_CASE("ld_y (8xy0)", "[cpu]")
{
    auto vx = GENERATE(range_i(0x0, 0xf));
    auto vy = GENERATE(range_i(0x0, 0xf));
    auto kk = GENERATE(take(10, random(0x00, 0xff)));

    const std::array<std::uint16_t, 2> instructions{
        b_or(0x6000, vy << 8, kk),     // ld_kk (loads kk into register vy)
        b_or(0x8000, vx << 8, vy << 4) // ld_y (loads vy into vx)
    };

    CPU cpu(make_rom(instructions.cbegin(), instructions.cend()), nullptr);

    REQUIRE_NOTHROW(cpu.step());
    REQUIRE(cpu.read_registers()[vy] == kk);

    REQUIRE_NOTHROW(cpu.step());
    REQUIRE(cpu.read_registers()[vx] == kk);
}

TEST_CASE("ld_addr (Annn)", "[cpu]")
{
    auto address = GENERATE(take(100, random(0x000, 0xfff)));

    const std::array<std::uint16_t, 1> instructions{
        b_or(0xA000, address) // ld_addr (loads address into VI)
    };

    CPU cpu(make_rom(instructions.cbegin(), instructions.cend()), nullptr);

    REQUIRE_NOTHROW(cpu.step());
    REQUIRE(cpu.read_vi() == address);
}

TEST_CASE("add_kk (7xkk)", "[cpu]")
{
    auto vx = GENERATE(range_i(0x0, 0xf));
    auto kk1 = GENERATE(take(10, random(0x00, 0xff)));
    auto kk2 = GENERATE(take(10, random(0x00, 0xff)));

    const std::array<std::uint16_t, 2> instructions{
        b_or(0x6000, vx << 8, kk1), // ld_kk (loads kk1 into register vx)
        b_or(0x7000, vx << 8, kk2)  // add_kk (adds kk2 to vx)
    };

    CPU cpu(make_rom(instructions.cbegin(), instructions.cend()), nullptr);

    REQUIRE_NOTHROW(cpu.step());
    REQUIRE(cpu.read_registers()[vx] == kk1);

    REQUIRE_NOTHROW(cpu.step());
    REQUIRE(cpu.read_registers()[vx] == ((kk1 + kk2) & 0xFF));
}

TEST_CASE("shr (8xy6)", "[cpu]")
{
    auto vx = GENERATE(range_i(0x0, 0xf));
    auto vy = GENERATE(range_i(0x0, 0xf));
    auto kk = GENERATE(take(10, random(0x00, 0xff)));

    const std::array<std::uint16_t, 2> instructions{
        b_or(0x6000, vy << 8, kk),     // ld_kk (loads kk into register vy)
        b_or(0x8006, vx << 8, vy << 4) // shr (shifts vy and stores the result in vx)
    };

    CPU cpu(make_rom(instructions.cbegin(), instructions.cend()), nullptr);

    REQUIRE_NOTHROW(cpu.step());
    REQUIRE(cpu.read_registers()[vy] == kk);

    REQUIRE_NOTHROW(cpu.step());
    if (vy != vx && vy != 0xf)
        REQUIRE(cpu.read_registers()[vy] == kk);
    REQUIRE(cpu.read_registers()[vx] == (kk >> 1));
    if (vx != 0xf)
        REQUIRE(cpu.read_registers()[0xf] == (kk & 1));
}

TEST_CASE("shl (8xyE)", "[cpu]")
{
    auto vx = GENERATE(range_i(0x0, 0xf));
    auto vy = GENERATE(range_i(0x0, 0xf));
    auto kk = GENERATE(take(10, random(0x00, 0xff)));

    // Too complicated for Catch to parse
    auto result = (kk << 1) & 0xff;

    const std::array<std::uint16_t, 2> instructions{
        b_or(0x6000, vy << 8, kk),     // ld_kk (loads kk into register vy)
        b_or(0x800E, vx << 8, vy << 4) // shl (shifts vy and stores the result in vx)
    };

    CPU cpu(make_rom(instructions.cbegin(), instructions.cend()), nullptr);

    REQUIRE_NOTHROW(cpu.step());
    REQUIRE(cpu.read_registers()[vy] == kk);

    REQUIRE_NOTHROW(cpu.step());
    if (vy != vx && vy != 0xf)
        REQUIRE(cpu.read_registers()[vy] == kk);
    REQUIRE(cpu.read_registers()[vx] == result);
    if (vx != 0xf)
        REQUIRE(cpu.read_registers()[0xf] == (kk & 0x80) >> 7);
}

TEST_CASE("or_y (8xy1)", "[cpu]")
{
    auto vx = GENERATE(range_i(0x0, 0xf));
    auto vy = GENERATE(range_i(0x0, 0xf));
    auto kk1 = GENERATE(take(10, random(0x00, 0xff)));
    auto kk2 = GENERATE(take(10, random(0x00, 0xff)));

    const std::array<std::uint16_t, 3> instructions{
        b_or(0x6000, vx << 8, kk1),    // ld_kk (loads kk1 into register vx)
        b_or(0x6000, vy << 8, kk2),    // ld_kk (loads kk2 into register vy)
        b_or(0x8001, vx << 8, vy << 4) // or_y (vx = vx, vy)
    };

    CPU cpu(make_rom(instructions.cbegin(), instructions.cend()), nullptr);

    REQUIRE_NOTHROW(cpu.step());
    REQUIRE(cpu.read_registers()[vx] == kk1);

    REQUIRE_NOTHROW(cpu.step());
    REQUIRE(cpu.read_registers()[vy] == kk2);

    REQUIRE_NOTHROW(cpu.step());
    REQUIRE(cpu.read_registers()[vy] == kk2);
    if (vx != vy)
        REQUIRE(cpu.read_registers()[vx] == (kk1 | kk2));
}

TEST_CASE("and_y (8xy2)", "[cpu]")
{
    auto vx = GENERATE(range_i(0x0, 0xf));
    auto vy = GENERATE(range_i(0x0, 0xf));
    auto kk1 = GENERATE(take(10, random(0x00, 0xff)));
    auto kk2 = GENERATE(take(10, random(0x00, 0xff)));

    const std::array<std::uint16_t, 3> instructions{
        b_or(0x6000, vx << 8, kk1),    // ld_kk (loads kk1 into register vx)
        b_or(0x6000, vy << 8, kk2),    // ld_kk (loads kk2 into register vy)
        b_or(0x8002, vx << 8, vy << 4) // and_y (vx = vx & vy)
    };

    CPU cpu(make_rom(instructions.cbegin(), instructions.cend()), nullptr);

    REQUIRE_NOTHROW(cpu.step());
    REQUIRE(cpu.read_registers()[vx] == kk1);

    REQUIRE_NOTHROW(cpu.step());
    REQUIRE(cpu.read_registers()[vy] == kk2);

    REQUIRE_NOTHROW(cpu.step());
    REQUIRE(cpu.read_registers()[vy] == kk2);
    if (vx != vy)
        REQUIRE(cpu.read_registers()[vx] == (kk1 & kk2));
}

TEST_CASE("xor_y (8xy3)", "[cpu]")
{
    auto vx = GENERATE(range_i(0x0, 0xf));
    auto vy = GENERATE(range_i(0x0, 0xf));
    auto kk1 = GENERATE(take(10, random(0x00, 0xff)));
    auto kk2 = GENERATE(take(10, random(0x00, 0xff)));

    const std::array<std::uint16_t, 3> instructions{
        b_or(0x6000, vx << 8, kk1),    // ld_kk (loads kk1 into register vx)
        b_or(0x6000, vy << 8, kk2),    // ld_kk (loads kk2 into register vy)
        b_or(0x8003, vx << 8, vy << 4) // xor_y (vx = vx ^ vy)
    };

    CPU cpu(make_rom(instructions.cbegin(), instructions.cend()), nullptr);

    REQUIRE_NOTHROW(cpu.step());
    REQUIRE(cpu.read_registers()[vx] == kk1);

    REQUIRE_NOTHROW(cpu.step());
    REQUIRE(cpu.read_registers()[vy] == kk2);

    REQUIRE_NOTHROW(cpu.step());
    if (vx == vy)
    {
        REQUIRE(cpu.read_registers()[vx] == 0);
    }
    else
    {
        REQUIRE(cpu.read_registers()[vy] == kk2);
        REQUIRE(cpu.read_registers()[vx] == (kk1 ^ kk2));
    }
}

TEST_CASE("add_y (8xy4)", "[cpu]")
{
    auto vx = GENERATE(range_i(0x0, 0xf));
    auto vy = GENERATE(range_i(0x0, 0xf));
    auto kk1 = GENERATE(take(10, random(0x00, 0xff)));
    auto kk2 = GENERATE(take(10, random(0x00, 0xff)));

    auto sum = vx == vy ? 2 * kk2 : kk1 + kk2;

    const std::array<std::uint16_t, 3> instructions{
        b_or(0x6000, vx << 8, kk1),    // ld_kk (loads kk1 into register vx)
        b_or(0x6000, vy << 8, kk2),    // ld_kk (loads kk2 into register vy)
        b_or(0x8004, vx << 8, vy << 4) // add_y (vx = vx + vy)
    };

    CPU cpu(make_rom(instructions.cbegin(), instructions.cend()), nullptr);

    REQUIRE_NOTHROW(cpu.step());
    REQUIRE(cpu.read_registers()[vx] == kk1);

    REQUIRE_NOTHROW(cpu.step());
    REQUIRE(cpu.read_registers()[vy] == kk2);

    REQUIRE_NOTHROW(cpu.step());
    if (vy != vx && vy != 0xf)
        REQUIRE(cpu.read_registers()[vy] == kk2);
    REQUIRE(cpu.read_registers()[vx] == (sum & 0xff));
    if (vx != 0xf)
        REQUIRE(cpu.read_registers()[0xf] == ((sum & 0x100) >> 8));
}

TEST_CASE("sub_y (8xy5)", "[cpu]")
{
    auto vx = GENERATE(range_i(0x0, 0xf));
    auto vy = GENERATE(range_i(0x0, 0xf));
    auto kk1 = GENERATE(take(10, random(0x00, 0xff)));
    auto kk2 = GENERATE(take(10, random(0x00, 0xff)));

    auto difference = vx == vy ? 0 : kk1 - kk2;

    const std::array<std::uint16_t, 3> instructions{
        b_or(0x6000, vx << 8, kk1),    // ld_kk (loads kk1 into register vx)
        b_or(0x6000, vy << 8, kk2),    // ld_kk (loads kk2 into register vy)
        b_or(0x8005, vx << 8, vy << 4) // sub_y (vx = vx - vy)
    };

    CPU cpu(make_rom(instructions.cbegin(), instructions.cend()), nullptr);

    REQUIRE_NOTHROW(cpu.step());
    REQUIRE(cpu.read_registers()[vx] == kk1);

    REQUIRE_NOTHROW(cpu.step());
    REQUIRE(cpu.read_registers()[vy] == kk2);

    REQUIRE_NOTHROW(cpu.step());
    if (vy != vx && vy != 0xf)
        REQUIRE(cpu.read_registers()[vy] == kk2);
    REQUIRE(cpu.read_registers()[vx] == (difference & 0xff));
    if (vx != 0xf)
        REQUIRE(cpu.read_registers()[0xf] == (difference < 0 ? 0 : 1));
}

TEST_CASE("subn_y (8xy7)", "[cpu]")
{
    auto vx = GENERATE(range_i(0x0, 0xf));
    auto vy = GENERATE(range_i(0x0, 0xf));
    auto kk1 = GENERATE(take(10, random(0x00, 0xff)));
    auto kk2 = GENERATE(take(10, random(0x00, 0xff)));

    auto difference = vx == vy ? 0 : kk2 - kk1;

    const std::array<std::uint16_t, 3> instructions{
        b_or(0x6000, vx << 8, kk1),    // ld_kk (loads kk1 into register vx)
        b_or(0x6000, vy << 8, kk2),    // ld_kk (loads kk2 into register vy)
        b_or(0x8007, vx << 8, vy << 4) // subn_y (vx = vy - vx)
    };

    CPU cpu(make_rom(instructions.cbegin(), instructions.cend()), nullptr);

    REQUIRE_NOTHROW(cpu.step());
    REQUIRE(cpu.read_registers()[vx] == kk1);

    REQUIRE_NOTHROW(cpu.step());
    REQUIRE(cpu.read_registers()[vy] == kk2);

    REQUIRE_NOTHROW(cpu.step());
    if (vy != vx && vy != 0xf)
        REQUIRE(cpu.read_registers()[vy] == kk2);
    REQUIRE(cpu.read_registers()[vx] == (difference & 0xff));
    if (vx != 0xf)
        REQUIRE(cpu.read_registers()[0xf] == (difference < 0 ? 0 : 1));
}

TEST_CASE("add_i (Fx1E)", "[cpu]")
{
    auto vx = GENERATE(range_i(0x0, 0xf));
    auto kk = GENERATE(take(10, random(0x00, 0xff)));
    auto nnn = GENERATE(take(10, random(0x000, 0xfff)));

    const std::array<std::uint16_t, 3> instructions{
        b_or(0x6000, vx << 8, kk), // ld_kk (loads kk into register vx)
        b_or(0xA000, nnn),         // ld_addr (loads nnn into VI)
        b_or(0xF01E, vx << 8)      // add_i (VI = VI + vx)
    };

    CPU cpu(make_rom(instructions.cbegin(), instructions.cend()), nullptr);

    REQUIRE_NOTHROW(cpu.step());
    REQUIRE(cpu.read_registers()[vx] == kk);

    REQUIRE_NOTHROW(cpu.step());
    REQUIRE(cpu.read_vi() == nnn);

    REQUIRE_NOTHROW(cpu.step());
    REQUIRE(cpu.read_vi() == (nnn + kk));
}

TEST_CASE("rnd (Cxkk)", "[cpu]")
{
    auto kk = GENERATE(take(100, random(0x01, 0xff)));

    std::array<std::uint16_t, 16> instructions;
    for (int i = 0; i < 16; ++i)
        instructions[i] = 0xC000 | (i << 8) | kk; // rnd (store (random byte & kk) to register i)

    CPU cpu(make_rom(instructions.cbegin(), instructions.cend()), nullptr);

    for (int i = 0; i < 16; ++i)
        REQUIRE_NOTHROW(cpu.step());

    const auto registers = cpu.read_registers();

    REQUIRE(std::all_of(registers.cbegin(), registers.cend(),
                        [kk](std::uint8_t x) { return (x & kk) == x; }));

    REQUIRE(std::any_of(registers.cbegin(), registers.cend(),
                        [&registers](std::uint8_t x) { return x != registers.front(); }));
}
