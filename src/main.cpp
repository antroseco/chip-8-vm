#include "cpu.hpp"
#include "rom.hpp"
#include "window.hpp"

#include <iostream>
#include <unistd.h>

int main(int argc, char* argv[])
{
    if (argc != 2)
    {
        std::cout << argv[0] << " [file]\n";
        return EXIT_FAILURE;
    }

    ScreenGuard Screen;
    Window Display(0, 0);

    auto ROM = LoadFile(argv[1]);

    if (!CheckROM(ROM))
    {
        std::cout << "Invalid ROM\n";
        return EXIT_FAILURE;
    }

    CPU Processor(ROM, &Display);

    Processor.Run();

    sleep(5);

    return 0;
}
