#include "cpu.hpp"

#include <algorithm>
#include <string>

constexpr std::array<uint8_t, 80> Font = {
    0xf0, 0x90, 0x90, 0x90, 0xf0, // 0
    0x20, 0x60, 0x20, 0x20, 0x70, // 1
    0xf0, 0x10, 0xf0, 0x80, 0xf0, // 2
    0xf0, 0x10, 0xf0, 0x10, 0xf0, // 3
    0x90, 0x90, 0xf0, 0x10, 0x10, // 4
    0xf0, 0x80, 0xf0, 0x10, 0xf0, // 5
    0xf0, 0x80, 0xf0, 0x90, 0xf0, // 6
    0xf0, 0x10, 0x20, 0x40, 0x40, // 7
    0xf0, 0x90, 0xf0, 0x90, 0xf0, // 8
    0xf0, 0x90, 0xf0, 0x10, 0xf0, // 9
    0xf0, 0x90, 0xf0, 0x90, 0x90, // A
    0xe0, 0x90, 0xe0, 0x90, 0xe0, // B
    0xf0, 0x90, 0x90, 0x90, 0xf0, // C
    0xe0, 0x90, 0x90, 0x90, 0xe0, // D
    0xf0, 0x80, 0xf0, 0x80, 0xf0, // E
    0xf0, 0x80, 0xf0, 0x80, 0x80  // F
};

CPU::CPU(const std::vector<uint8_t>& ROM, Window* Display) : IP(ROM.data()), Display(Display)
{
    std::copy(Font.cbegin(), Font.cend(), Memory.begin());
    std::copy_n(ROM.cbegin(), std::min(0xDFFul, ROM.size()), std::next(Memory.begin(), 0x200));
}

bool CPU::Step()
{
    const bool not_finished = Execute();

    if (UpdatePC)
        AdvancePC(1);
    else
        UpdatePC = true;

    return not_finished;
}

void CPU::Run()
{
    while (Step())
        ;
}

const std::array<uint8_t, 0x1000>& CPU::read_memory() const noexcept
{
    return Memory;
}

const std::stack<uint16_t>& CPU::read_stack() const noexcept
{
    return Stack;
}

const std::array<uint8_t, 16>& CPU::read_registers() const noexcept
{
    return V;
}

uint16_t CPU::read_vi() const noexcept
{
    return VI;
}

uint16_t CPU::read_pc() const noexcept
{
    return PC;
}

bool CPU::Execute()
{
    switch (IP.opcode())
    {
    case 0x00E0:
        cls();
        return true;
    case 0x00EE:
        ret();
        return true;
    case 0x1000:
        return jp();
    case 0x2000:
        call();
        return true;
    case 0x3000:
        se_x_kk();
        return true;
    case 0x4000:
        sne_x_kk();
        return true;
    case 0x5000:
        se_x_y();
        return true;
    case 0x6000:
        ld_kk();
        return true;
    case 0x7000:
        add_kk();
        return true;
    case 0x8000:
        ld_y();
        return true;
    case 0x8001:
        or_y();
        return true;
    case 0x8002:
        and_y();
        return true;
    case 0x8003:
        xor_y();
        return true;
    case 0x8004:
        add_y();
        return true;
    case 0x8005:
        sub_y();
        return true;
    case 0x8006:
        shr();
        return true;
    case 0x8007:
        subn_y();
        return true;
    case 0x800e:
        shl();
        return true;
    case 0x9000:
        sne_x_y();
        return true;
    case 0xA000:
        ld_addr();
        return true;
    case 0xB000:
        return jp_v0();
    case 0xC000:
        rnd();
        return true;
    case 0xD000:
        drw();
        return true;
    case 0xF01E:
        add_i();
        return true;
    case 0xF029:
        ld_digit();
        return true;
    case 0xF033:
        str_bcd();
        return true;
    case 0xF055:
        str_vx();
        return true;
    case 0xF065:
        ld_vx();
        return true;
    }

    throw std::logic_error("Opcode " + std::to_string(IP.opcode()) + " not implemented");
}

void CPU::AdvancePC(const uint_fast16_t Instructions)
{
    SetPC(PC + Instructions * 2);
}

void CPU::SetPC(const uint16_t Address)
{
    if (Address >= Memory.size() - 1)
        throw std::out_of_range(std::to_string(Address));

    PC = Address;
    IP.read(std::next(Memory.data(), PC));
}

bool CPU::jp()
{
    /*
    * 1nnn - JP addr
    * Jump to location nnn.
    *
    * The interpreter sets the program counter to nnn.
    */

    // Check if we are stuck in a loop
    if (PC == IP.nnn())
        return false;

    SetPC(IP.nnn());
    UpdatePC = false;

    return true;
}

void CPU::call()
{
    /*
    * 2nnn - CALL addr
    * Call subroutine at nnn.
    *
    * The interpreter increments the stack pointer, then puts the current PC
    * on the top of the stack. The PC is then set to nnn.
    */

    // The stack should only contain 16 addresses
    if (Stack.size() >= 16)
        throw std::runtime_error("Stack overflow");

    Stack.push(PC);
    jp();
}

void CPU::ret()
{
    /*
    * 00EE - RET
    * Return from a subroutine.
    *
    * The interpreter sets the program counter to the address at the top
    * of the stack, then subtracts 1 from the stack pointer.
    */

    if (Stack.empty())
        throw std::runtime_error("Stack underflow");

    SetPC(Stack.top() + 2);
    UpdatePC = false;

    Stack.pop();
}

void CPU::se_x_kk()
{
    /*
    * 3xkk - SE Vx, byte
    * Skip next instruction if Vx = kk.
    *
    * The interpreter compares register Vx to kk, and if they are equal,
    * increments the program counter by 2.
    */

    if (V[IP.x()] == IP.kk())
    {
        AdvancePC(2);
        UpdatePC = false;
    }
}

void CPU::sne_x_kk()
{
    /*
    * 4xkk - SNE Vx, byte
    * Skip next instruction if Vx != kk.
    *
    * The interpreter compares register Vx to kk, and if they are not equal,
    * increments the program counter by 2.
    */

    if (V[IP.x()] != IP.kk())
    {
        AdvancePC(2);
        UpdatePC = false;
    }
}

void CPU::se_x_y()
{
    /*
    * 5xy0 - SE Vx, Vy
    * Skip next instruction if Vx = Vy.
    *
    * The interpreter compares register Vx to register Vy, and if they are
    * equal, increments the program counter by 2.
    */

    if (V[IP.x()] == V[IP.y()])
    {
        AdvancePC(2);
        UpdatePC = false;
    }
}

void CPU::ld_kk() noexcept
{
    /*
    * 6xkk - LD Vx, byte
    * Set Vx = kk.
    *
    * The interpreter puts the value kk into register Vx.
    */

    V[IP.x()] = IP.kk();
}

void CPU::add_kk() noexcept
{
    /*
    * 7xkk - ADD Vx, byte
    * Set Vx = Vx + kk.
    *
    * Adds the value kk to the value of register Vx, then stores the result in Vx.
    */

    V[IP.x()] += IP.kk();
}

void CPU::ld_y() noexcept
{
    /*
    * 8xy0 - LD Vx, Vy
    * Set Vx = Vy.
    *
    * Stores the value of register Vy in register Vx.
    */

    V[IP.x()] = V[IP.y()];
}

void CPU::or_y() noexcept
{
    /*
    * 8xy1 - OR Vx, Vy
    * Set Vx = Vx OR Vy.
    *
    * Performs a bitwise OR on the values of Vx and Vy, then stores the
    * result in Vx.
    */

    V[IP.x()] |= V[IP.y()];
}

void CPU::and_y() noexcept
{
    /*
    * 8xy2 - AND Vx, Vy
    * Set Vx = Vx AND Vy.
    *
    * Performs a bitwise AND on the values of Vx and Vy, then stores the
    * result in Vx.
    */

    V[IP.x()] &= V[IP.y()];
}

void CPU::xor_y() noexcept
{
    /*
    * 8xy3 - XOR Vx, Vy
    * Set Vx = Vx XOR Vy.
    *
    * Performs a bitwise exclusive OR on the values of Vx and Vy, then
    * stores the result in Vx.
    */

    V[IP.x()] ^= V[IP.y()];
}

void CPU::add_y() noexcept
{
    /*
    * 8xy4 - ADD Vx, Vy
    * Set Vx = Vx + Vy, set VF = carry.
    *
    * The values of Vx and Vy are added together. If the result is greater
    * than 8 bits (i.e., > 255,) VF is set to 1, otherwise 0. Only the
    * lowest 8 bits of the result are kept, and stored in Vx.
    */

    const uint_fast16_t Result = V[IP.x()] + V[IP.y()];
    V[IP.x()] = Result & 0xFF;
    VF = Result > 0xFF ? 1 : 0;
}

void CPU::sub_y() noexcept
{
    /*
    * 8xy5 - SUB Vx, Vy
    * Set Vx = Vx - Vy, set VF = NOT borrow.
    *
    * If Vx > Vy, then VF is set to 1, otherwise 0. Then Vy is subtracted
    * from Vx, and the results stored in Vx.
    */

    V[IP.x()] -= V[IP.y()];
    VF = V[IP.x()] > V[IP.y()] ? 1 : 0;
}

void CPU::shr() noexcept
{
    /*
    * 8xy6 - SHR Vx, Vy
    * Set Vx = Vy SHR 1.
    *
    * Store the value of register VY shifted right one bit in register VX.
    * Set register VF to the least significant bit prior to the shift.
    */

    // Make a copy of the data in case it's stored in VF
    const uint8_t data = V[IP.y()];

    VF = data & 0x01;
    V[IP.x()] = data >> 1;
}

void CPU::shl() noexcept
{
    /*
    * 8xyE - SHL Vx, Vy
    * Set Vx = Vy SHL 1.
    *
    * Store the value of register VY shifted left one bit in register VX.
    * Set register VF to the most significant bit prior to the shift.
    */

    // Make a copy of the data in case it's stored in VF
    const uint8_t data = V[IP.y()];

    VF = (data & 0x80) >> 7;
    V[IP.x()] = data << 1;
}

void CPU::subn_y() noexcept
{
    /*
    * 8xy7 - SUBN Vx, Vy
    * Set Vx = Vy - Vx, set VF = NOT borrow.
    *
    * If Vy > Vx, then VF is set to 1, otherwise 0. Then Vx is subtracted
    * from Vy, and the results stored in Vx.
    */

    VF = V[IP.y()] > V[IP.x()] ? 1 : 0;
    V[IP.x()] = V[IP.y()] - V[IP.x()];
}

void CPU::sne_x_y()
{
    /*
    * 9xy0 - SNE Vx, Vy
    * Skip next instruction if Vx != Vy.
    *
    * The values of Vx and Vy are compared, and if they are not equal, the
    * program counter is increased by 2.
    */

    if (V[IP.x()] != V[IP.y()])
    {
        AdvancePC(2);
        UpdatePC = false;
    }
}

void CPU::ld_addr() noexcept
{
    /*
    * Annn - LD I, addr
    * Set I = nnn.
    *
    * The value of register I is set to nnn.
    */

    VI = IP.nnn();
}

bool CPU::jp_v0()
{
    /*
    * Bnnn - JP V0, addr
    * Jump to location nnn + V0.
    *
    * The program counter is set to nnn plus the value of V0.
    */

    // Check if we are stuck in a loop
    if (PC == IP.nnn() + V[0])
        return false;

    SetPC(IP.nnn() + V[0]);
    UpdatePC = false;

    return true;
}

void CPU::rnd()
{
    /*
    * Cxkk - RND Vx, byte
    * Set Vx = random byte AND kk.
    *
    * The interpreter generates a random number from 0 to 255, which is then
    * ANDed with the value kk. The results are stored in Vx. See instruction
    * 8xy2 for more information on AND.
    */

    V[IP.x()] = Distribution(Generator) & IP.kk();
}

void CPU::drw()
{
    /*
    * Dxyn - DRW Vx, Vy, nibble
    * Display n-byte sprite starting at memory location I at (Vx, Vy),
    * setVF = collision.
    *
    * The interpreter reads n bytes from memory, starting at the address
    * stored in I. These bytes are then displayed as sprites on screen at
    * coordinates (Vx, Vy). Sprites are XORed onto the existing screen. If
    * this causes any pixels to be erased, VF is set to 1, otherwise it is
    * set to 0. If the sprite is positioned so part of it is outside the
    * coordinates of the display, it wraps around to the opposite side of the
    * screen.
    */

    auto address = std::next(Memory.cbegin(), VI);

    std::vector<uint8_t> Sprite;
    std::copy_n(address, IP.n(), std::back_inserter(Sprite));

    if (Display != nullptr)
    {
        VF = Display->DrawSprite(Sprite, V[IP.x()], V[IP.y()]);
        Display->Refresh();
    }
}

void CPU::cls()
{
    /*
    * 00E0 - CLS
    * Clear the display.
    */

    if (Display != nullptr)
    {
        Display->Clear();
        Display->Refresh();
    }
}

void CPU::add_i() noexcept
{
    /*
    * Fx1E - ADD I, Vx
    * Set I = I + Vx.
    *
    * The values of I and Vx are added, and the results are stored in I.
    */

    VI += V[IP.x()];
}

void CPU::str_vx()
{
    /*
    * Fx55 - LD [I], Vx
    * Store registers V0 through Vx in memory starting at location I.
    *
    * The interpreter copies the values of registers V0 through Vx into
    * memory, starting at the address in I.
    */

    if (VI + IP.x() >= Memory.size())
        throw std::out_of_range(std::to_string(VI + IP.x()));

    auto address = std::next(Memory.begin(), VI);

    std::copy_n(V.cbegin(), IP.x() + 1, address);

    // Undocumented
    VI += IP.x() + 1;
}

void CPU::ld_vx()
{
    /*
    * Fx65 - LD Vx, [I]
    * Read registers V0 through Vx from memory starting at location I.
    *
    * The interpreter reads values from memory starting at location I into
    * registers V0 through Vx.
    */

    if (VI + IP.x() >= Memory.size())
        throw std::out_of_range(std::to_string(VI + IP.x()));

    auto address = std::next(Memory.cbegin(), VI);

    std::copy_n(address, IP.x() + 1, V.begin());

    // Undocumented
    VI += IP.x() + 1;
}

void CPU::str_bcd()
{
    /*
    * Fx33 - LD B, Vx
    * Store BCD representation of Vx in memory locations I, I+1, and I+2.
    *
    * The interpreter takes the decimal value of Vx, and places the hundreds
    * digit in memory at location in I, the tens digit at location I+1, and
    * the ones digit at location I+2.
    */

    const uint8_t value = V[IP.x()];

    Memory.at(VI + 0) = value / 100;
    Memory.at(VI + 1) = (value / 10) % 10;
    Memory.at(VI + 2) = value % 10;
}

void CPU::ld_digit()
{
    /*
    * Fx29 - LD F, Vx
    * Set I = location of sprite for digit Vx.
    *
    * The value of I is set to the location for the hexadecimal sprite
    * corresponding to the value of Vx.
    */

    VI = IP.x() * 5;
}
