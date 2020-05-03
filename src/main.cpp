#include "cpu.hpp"
#include "graphics.hpp"
#include "rom.hpp"

#include <SFML/Window.hpp>

#include <future>
#include <iostream>
#include <thread>

int main(int argc, char* argv[])
{
    if (argc != 2)
    {
        std::cout << argv[0] << " [file]\n";
        return EXIT_FAILURE;
    }

    try
    {
        auto ROM = LoadFile(argv[1]);

        if (!CheckROM(ROM))
        {
            std::cout << "Invalid ROM\n";
            return EXIT_FAILURE;
        }

        const sf::VideoMode resolution{Frame::Columns * 10, Frame::Lines * 10};
        sf::RenderWindow window(resolution, "CHIP-8 Virtual Machine");

        std::cout << "Using OpenGL " << window.getSettings().majorVersion
                  << "." << window.getSettings().minorVersion << std::endl;

        window.setFramerateLimit(60);
        Frame::prepareTarget(window);

        Frame frame;
        Keyboard keyboard{window};
        CPU cpu{ROM, &frame, &keyboard};

        std::promise<void> stop_token;
        std::thread cpu_thread{&CPU::run, &cpu, stop_token.get_future()};

        sf::Clock clock;
        int frame_count = 0;

        while (window.isOpen())
        {
            bool force_redraw = false;

            // Event processing
            sf::Event event;
            while (window.pollEvent(event))
            {
                if (event.type == sf::Event::Closed)
                {
                    window.close();

                    stop_token.set_value();
                    cpu_thread.join();

                    return EXIT_SUCCESS;
                }
                else if (event.type == sf::Event::Resized)
                {
                    force_redraw = true;
                }
                else if (event.type == sf::Event::KeyPressed)
                {
                    keyboard.register_keypress(event.key, true);
                }
                else if (event.type == sf::Event::KeyReleased)
                {
                    keyboard.register_keypress(event.key, false);
                }

                // TODO: Process more event types
            }

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
