#include "cpu.hpp"
#include "graphics.hpp"
#include "rom.hpp"

#include <SFML/Window.hpp>

#include <iostream>

#ifdef FUZZING

extern "C" int LLVMFuzzerTestOneInput(const std::uint8_t* Data, std::size_t Size)
{
    std::vector<std::uint8_t> ROM{Data, Data + Size};
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
        bool not_done = true;

        const sf::VideoMode resolution{Frame::Columns * 10, Frame::Lines * 10};
        sf::RenderWindow window(resolution, "CHIP-8 Virtual Machine");

        std::cout << "Using OpenGL " << window.getSettings().majorVersion
                  << "." << window.getSettings().minorVersion << std::endl;

        window.setFramerateLimit(60);
        Frame::prepareTarget(window);

        Frame frame;
        CPU cpu(ROM, &frame);

        sf::Clock clock;
        int frame_count = 0;

        while (window.isOpen())
        {
            bool force_redraw = false;

            // Event processing
            sf::Event event;
            while (window.pollEvent(event))
            {
                // Request for closing the window
                if (event.type == sf::Event::Closed)
                {
                    window.close();
                    return EXIT_SUCCESS;
                }
                else if (event.type == sf::Event::Resized)
                {
                    force_redraw = true;
                }

                // TODO: Process more event types
            }

            // TODO: Untie CPU clockspeed from framerate
            for (int i = 0; i < 9 && not_done; ++i)
                not_done = cpu.Step();

            frame.render(window, force_redraw);
            window.display();

            if (frame_count < 100)
            {
                frame_count++;
            }
            else
            {
                sf::Time elapsed = clock.getElapsedTime();
                std::cout << (float)elapsed.asMilliseconds() / frame_count << " ms (" << frame_count / elapsed.asSeconds() << " fps)\n";

                frame_count = 0;
                clock.restart();
            }
        }
    }
    catch (const std::exception& e)
    {
        std::cout << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

#endif
