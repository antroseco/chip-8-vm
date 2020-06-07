#include "instruction.hpp"

#include <cassert>
#include <stdexcept>
#include <string>

Instruction::Instruction(const std::uint8_t* address)
{
    read(address);
}

void Instruction::read(const std::uint8_t* address)
{
    assert(address != nullptr);

    /*
    * Read byte-by-byte to avoid unaligned memory accesses,
    * which are undefined behaviour (shorts are aligned to an even address)
    * Assume that address contains data in a big endian format
    */
    raw = address[0] << 8 | address[1];
}

std::uint_fast8_t Instruction::group() const noexcept
{
    return raw >> 12;
}

std::uint_fast8_t Instruction::x() const noexcept
{
    return (raw & 0x0F00) >> 8;
}

std::uint_fast8_t Instruction::y() const noexcept
{
    return (raw & 0x00F0) >> 4;
}

std::uint_fast8_t Instruction::n() const noexcept
{
    return (raw & 0x000F);
}

std::uint_fast8_t Instruction::kk() const noexcept
{
    return (raw & 0x00FF);
}

std::uint_fast16_t Instruction::nnn() const noexcept
{
    return (raw & 0x0FFF);
}
