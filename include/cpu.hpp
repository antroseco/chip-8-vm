#pragma once

#include "instruction.hpp"
#include "timer.hpp"
#include "utility.hpp"

#include <array>
#include <cstddef>
#include <cstdint>
#include <future>
#include <random>

class Frame;
class Keyboard;

/*
http://devernay.free.fr/hacks/chip8/C8TECH10.HTM#memmap

+---------------+= 0xFFF (4095) End of Chip-8 RAM
|               |
|               |
|               |
|               |
|               |
| 0x200 to 0xFFF|
|     Chip-8    |
| Program / Data|
|     Space     |
|               |
|               |
|               |
+- - - - - - - -+= 0x600 (1536) Start of ETI 660 Chip-8 programs
|               |
|               |
|               |
+---------------+= 0x200 (512) Start of most Chip-8 programs
| 0x000 to 0x1FF|
| Reserved for  |
|  interpreter  |
+---------------+= 0x000 (0) Start of Chip-8 RAM
*/

class CPU
{
    bool Modern;

    std::array<std::uint8_t, 0x1000> Memory = {}; // 4096 bytes of RAM
    std::uint16_t VI = 0;                         // 16-bit address register

    std::array<std::uint_fast16_t, 12> Stack; // Stack, up to 12 16-bit addresses
    std::size_t SP = 0;                       // Stack Pointer

    std::array<std::uint8_t, 16> V = {}; // 16 8-bit data registers (V0, V1, ..., VF)
    std::uint8_t& VF = V.back();         // Flag register (equivalent to V[0xF])

    std::uint16_t PC = 0x200; // Program Counter
    Instruction IP;           // Instruction Pointer

    Timer DT; // Delay Timer

    Frame* const Display;
    Keyboard* const Input;

    std::mt19937 Generator;

    bool UpdatePC = true;

    bool Execute();
    void SkipInstructions(int Instructions);
    void SetPC(std::uint16_t Address);

    // Instruction set
    bool jp();
    bool jp_v0();
    void call();
    void ret();

    void se_x_kk();
    void se_x_y();
    void sne_x_kk();
    void sne_x_y();

    void ld_kk() noexcept;
    void ld_y() noexcept;
    void ld_addr() noexcept;

    void add_kk() noexcept;
    void shr() noexcept;
    void shl() noexcept;

    void or_y() noexcept;
    void and_y() noexcept;
    void xor_y() noexcept;
    void add_y() noexcept;
    void sub_y() noexcept;
    void subn_y() noexcept;

    void add_i() noexcept;

    void rnd();
    void drw(); // TODO: test
    void cls(); // TODO: test

    void str_vx();   // TODO: test
    void ld_vx();    // TODO: test
    void str_bcd();  // TODO: test
    void ld_digit(); // TODO: test

    void ld_dt() noexcept;  // TODO: test
    void set_dt() noexcept; // TODO: test

    void skp_key() noexcept;  // TODO: test
    void sknp_key() noexcept; // TODO: test
    void ld_key() noexcept;   // TODO: test

public:
    CPU() = delete;
    CPU(byte_view ROM, bool ModernBehaviour = false, Frame* Display = nullptr, Keyboard* Input = nullptr);
    bool step();
    void run_at(const std::future<void>& stop_token, std::size_t target_frequency);

    byte_view read_memory() const noexcept;
    byte_view read_registers() const noexcept;
    data_view<std::uint_fast16_t> read_stack() const noexcept;
    std::uint16_t read_vi() const noexcept;
    std::uint16_t read_pc() const noexcept;
};
