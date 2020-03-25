#pragma once

#include "instruction.hpp"
#include "timer.hpp"
#include "window.hpp"

#include <array>
#include <random>
#include <stack>

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
    std::array<uint8_t, 0x1000> Memory = {0}; // 4096 bytes of RAM
    std::stack<uint16_t> Stack;               // Stack, up to 16 16-bit addresses

    std::array<uint8_t, 16> V = {0}; // 16 8-bit data registers (V0, V1, ..., VF)
    uint8_t& VF = V.back();          // Flag register (equivalent to V[0xF])
    uint16_t VI = 0;                 // 16-bit address register
    uint16_t PC = 0x200;             // Program Counter
    Instruction IP;                  // Instruction Pointer

    Timer DT; // Delay Timer

    Window* const Display;

    std::random_device Generator;                        // Random number generator
    std::uniform_int_distribution<uint8_t> Distribution; // Random number distribution

    bool UpdatePC = true;

    bool Execute();
    void AdvancePC(const uint_fast16_t Instructions);
    void SetPC(const uint16_t Address);

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

public:
    CPU() = delete;
    CPU(const std::vector<uint8_t>& ROM, Window* Display);
    bool Step();
    void Run();

    const std::array<uint8_t, 0x1000>& read_memory() const noexcept;
    const std::stack<uint16_t>& read_stack() const noexcept;
    const std::array<uint8_t, 16>& read_registers() const noexcept;
    uint16_t read_vi() const noexcept;
    uint16_t read_pc() const noexcept;
};
