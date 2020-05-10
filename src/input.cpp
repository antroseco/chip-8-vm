#include "input.hpp"

#include <SFML/Window.hpp>

#include <algorithm>
#include <iterator>
#include <optional>

static std::optional<int> Map(sf::Keyboard::Key key) noexcept
{
    switch (key)
    {
    case sf::Keyboard::Numpad0:
        return 0x0;
    case sf::Keyboard::Numpad1:
        return 0x1;
    case sf::Keyboard::Numpad2:
        return 0x2;
    case sf::Keyboard::Numpad3:
        return 0x3;
    case sf::Keyboard::Numpad4:
        return 0x4;
    case sf::Keyboard::Numpad5:
        return 0x5;
    case sf::Keyboard::Numpad6:
        return 0x6;
    case sf::Keyboard::Numpad7:
        return 0x7;
    case sf::Keyboard::Numpad8:
        return 0x8;
    case sf::Keyboard::Numpad9:
        return 0x9;
    case sf::Keyboard::A:
        return 0xA;
    case sf::Keyboard::B:
        return 0xB;
    case sf::Keyboard::C:
        return 0xC;
    case sf::Keyboard::D:
        return 0xD;
    case sf::Keyboard::E:
        return 0xE;
    case sf::Keyboard::F:
        return 0xF;
    default:
        return std::nullopt;
    }
}

Keyboard::Keyboard(sf::Window& window)
{
    window.setKeyRepeatEnabled(false);
}

void Keyboard::register_keypress(sf::Event::KeyEvent event, bool state) noexcept
{
    const std::optional<int> key = Map(event.code);

    // Ignore keys we don't care about
    if (key.has_value())
        keys.at(key.value()).store(state, std::memory_order_relaxed);
}

bool Keyboard::query_key(int key) const noexcept
{
    return keys.at(key).load(std::memory_order_relaxed);
}

std::optional<int> Keyboard::query_any() const noexcept
{
    const auto result = std::find_if(keys.cbegin(), keys.cend(),
                                     [](const std::atomic_bool& x) { return x.load(std::memory_order_relaxed); });

    if (result != keys.cend())
        return std::distance(keys.cbegin(), result);
    else
        return std::nullopt;
}
