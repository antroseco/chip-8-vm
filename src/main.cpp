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
