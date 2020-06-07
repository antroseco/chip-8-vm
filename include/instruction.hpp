#pragma once

#include <cassert>
#include <cstdint>

struct Instruction
{
    /*
    * Variable convention:
    * o => opcode (at least the first nibble)
    * x => first operand (second nibble)
    * y => second operand (third nibble)
    * n => suffix/nibble (fourth nibble)
    * nnn => 12-bit address (3 lower nibbles)
    * kk => 8-bit constant (low byte)
    *
    * Possible instruction formats:
    * oooo (0 operands)
    * oxoo (1 operand)
    * oxyo (2 operands)
    * onnn (address)
    * oxkk (1 operand, 1 constant)
    * oxyn (2 operands, 1 constant)
    */

    static constexpr int width = 2;

    std::uint_fast16_t raw = 0;

    constexpr Instruction() noexcept = default;

    constexpr explicit Instruction(std::uint16_t instruction) noexcept : raw(instruction) {}

    constexpr explicit Instruction(const std::uint8_t* address) noexcept
    {
        read(address);
    }

    constexpr void read(const std::uint8_t* address) noexcept
    {
        assert(address != nullptr);

        /*
        * Read byte-by-byte to avoid unaligned memory accesses,
        * which are undefined behaviour (shorts are aligned to an even address)
        * Assume that address contains data in a big endian format
        */
        raw = address[0] << 8 | address[1];
    }

    [[nodiscard]] constexpr std::uint_fast8_t group() const noexcept
    {
        return raw >> 12;
    }

    [[nodiscard]] constexpr std::uint_fast8_t x() const noexcept
    {
        return (raw & 0x0F00) >> 8;
    }

    [[nodiscard]] constexpr std::uint_fast8_t y() const noexcept
    {
        return (raw & 0x00F0) >> 4;
    }

    [[nodiscard]] constexpr std::uint_fast8_t n() const noexcept
    {
        return raw & 0x000F;
    }

    [[nodiscard]] constexpr std::uint_fast8_t kk() const noexcept
    {
        return raw & 0x00FF;
    }

    [[nodiscard]] constexpr std::uint_fast16_t nnn() const noexcept
    {
        return raw & 0x0FFF;
    }
};
