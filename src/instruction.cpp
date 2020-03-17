#include "instruction.hpp"

#include <stdexcept>

Instruction::Instruction(const uint16_t Instruction) : raw(Instruction) {}

uint_fast16_t Instruction::opcode() const
{
    // 00E0 or 00EE
    if (raw == 0x00E0)
        return 0x00E0;
    else if (raw == 0x00EE)
        return 0x00EE;

    // First nibble
    uint_fast16_t value = raw & 0xF000;

    switch (value >> 12)
    {
    case 0x5: // 5xy0
    case 0x9: // 9xy0
        if (n() != 0)
            throw std::invalid_argument(std::to_string(raw));
    // Fallthrough
    case 0x0: // 0nnn
    case 0x1: // 1nnn
    case 0x2: // 2nnn
    case 0x3: // 3xkk
    case 0x4: // 4xkk
    case 0x6: // 6xkk
    case 0x7: // 7xkk
    case 0xA: // Annn
    case 0xB: // Bnnn
    case 0xC: // Cxkk
    case 0xD: // Dxyn
        return value;
    case 0x8: // 8xy? [0-7, E]
        if (n() > 7 && n() != 0xE)
            throw std::invalid_argument(std::to_string(raw));
        return value | n();
    case 0xE: // Ex9E or ExA1
        if (kk() != 0x9E && kk() != 0xA1)
            throw std::invalid_argument(std::to_string(raw));
        return value | kk();
    case 0xF: // Fx??
        switch (kk())
        {
        case 0x07:
        case 0x0A:
        case 0x15:
        case 0x18:
        case 0x1E:
        case 0x29:
        case 0x33:
        case 0x55:
        case 0x65:
            return value | kk();
        }
    }

    throw std::invalid_argument(std::to_string(raw));
}

uint_fast8_t Instruction::x() const noexcept
{
    return (raw & 0x0F00) >> 8;
}

uint_fast8_t Instruction::y() const noexcept
{
    return (raw & 0x00F0) >> 4;
}

uint_fast8_t Instruction::n() const noexcept
{
    return (raw & 0x000F);
}

uint_fast8_t Instruction::kk() const noexcept
{
    return (raw & 0x00FF);
}

uint_fast16_t Instruction::nnn() const noexcept
{
    return (raw & 0x0FFF);
}
