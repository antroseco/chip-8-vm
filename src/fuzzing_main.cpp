#include "cpu.hpp"

#include <cstdint>
#include <vector>

extern "C" int LLVMFuzzerTestOneInput(const std::uint8_t* data, std::size_t size)
{
    std::vector<std::uint8_t> ROM{data, data + size};
    CPU processor{ROM};

    try
    {
        for (int i = 0; processor.step() && i < 10000; ++i)
            ;
    }
    catch (std::logic_error)
    {
        // Invalid opcodes
    }
    catch (std::runtime_error)
    {
        // Stack over/underflow, invalid key
    }
    catch (std::out_of_range)
    {
        // Invalid memory address
    }
    catch (std::invalid_argument)
    {
        // Invalid opcode
    }

    return 0; // Non-zero return values are reserved for future use.
}
