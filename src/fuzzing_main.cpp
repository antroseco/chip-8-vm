#include "cpu.hpp"
#include "graphics.hpp"
#include "utility.hpp"

#include <cstddef>
#include <cstdint>
#include <stdexcept>

extern "C" int LLVMFuzzerTestOneInput(const std::uint8_t* data, std::size_t size)
{
    const byte_view ROM{data, size};
    Frame frame;
    CPU processor{ROM, false, &frame};

    try
    {
        for (int i = 0; processor.step() && i < 10000; ++i)
            continue;
    }
    catch (std::logic_error)
    {
        // Invalid opcodes
    }
    catch (std::runtime_error)
    {
        // Invalid key
    }
    catch (std::out_of_range)
    {
        // Invalid memory address, Stack over/underflow
    }

    return 0; // Non-zero return values are reserved for future use.
}
