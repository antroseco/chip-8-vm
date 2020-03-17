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

    Instruction() = delete;
    Instruction(const uint16_t);

    const uint16_t raw;

    uint_fast16_t opcode() const;
    uint_fast8_t x() const noexcept;
    uint_fast8_t y() const noexcept;
    uint_fast8_t n() const noexcept;
    uint_fast8_t kk() const noexcept;
    uint_fast16_t nnn() const noexcept;
};
