#pragma once

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

    std::uint16_t raw;

    constexpr Instruction() : raw(0){};
    constexpr explicit Instruction(std::uint16_t instruction) : raw(instruction){};
    explicit Instruction(const std::uint8_t* address);

    void read(const std::uint8_t* address);

    std::uint_fast16_t opcode() const;
    std::uint_fast8_t x() const noexcept;
    std::uint_fast8_t y() const noexcept;
    std::uint_fast8_t n() const noexcept;
    std::uint_fast8_t kk() const noexcept;
    std::uint_fast16_t nnn() const noexcept;
};
