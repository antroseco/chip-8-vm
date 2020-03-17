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
        return 1;
    }

    ScreenGuard Screen;
    Window Display(0, 0);

    auto File = LoadFile(argv[1]);
    auto ROM = ParseROM(File);

    CPU Processor(&Display, std::move(ROM));

    Processor.Run();

    sleep(5);

    return 0;
}
