#include "cpu.hpp"
#include "graphics.hpp"
#include "input.hpp"
#include "rom.hpp"

#include "CLI11.hpp"
#include <SFML/Graphics.hpp>
#include <SFML/System.hpp>
#include <SFML/Window.hpp>

#include <cstdlib>
#include <exception>
#include <future>
#include <iostream>
#include <string>
#include <thread>

int main(int argc, char* argv[])
{
    CLI::App app{"CHIP-8 interpreter as implemented on the COSMAC VIP"};

    std::string rom_path;
    app.add_option("rom", rom_path, "ROM to execute")->required()->check(CLI::ExistingFile);

    bool modern_behaviour = false;
    app.add_flag("-m,--modern", modern_behaviour, "Use modern shifting behaviour (8xy6 & 8xyE)");

    CLI11_PARSE(app, argc, argv);

    try
    {
        auto ROM = LoadFile(rom_path);

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
        CPU cpu{ROM, modern_behaviour, &frame, &keyboard};

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
