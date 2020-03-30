#include "cpu.hpp"
#include "rom.hpp"
#include "window.hpp"

#include <iostream>
#include <unistd.h>

#ifdef FUZZING

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* Data, size_t Size)
{
    std::vector<uint8_t> ROM{Data, Data + Size};
    CPU Processor{ROM, nullptr};
    try
    {
        for (int i = 0; Processor.Step() && i < 1000; ++i)
            ;
    }
    catch (std::logic_error)
    {
        // Invalid opcodes
    }
    catch (std::runtime_error)
    {
        // Stack over/underflow
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

#else

int main(int argc, char* argv[])
{
    if (argc != 2)
    {
        std::cout << argv[0] << " [file]\n";
        return EXIT_FAILURE;
    }

    auto ROM = LoadFile(argv[1]);

    if (!CheckROM(ROM))
    {
        std::cout << "Invalid ROM\n";
        return EXIT_FAILURE;
    }

    try
    {
        ScreenGuard Screen;
        Window Display(0, 0);

        CPU Processor(ROM, &Display);

        Processor.Run();

        Display.WriteString(31, 0, "Done");
        Display.Refresh();

        sleep(5);
    }
    catch (const std::exception& e)
    {
        std::cout << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

#endif
