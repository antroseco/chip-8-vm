#include "graphics.hpp"

#include <SFML/Graphics.hpp>

#include <cstdint>
#include <limits>

[[nodiscard]] static constexpr std::uint64_t rotr(std::uint64_t x, std::size_t s) noexcept
{
    constexpr auto digits = std::numeric_limits<std::uint64_t>::digits;

    const std::size_t n = s % digits;
    return (x >> n) | (x << ((digits - n) % digits));
}

bool Frame::drawSprite(byte_view sprite, std::size_t x, std::size_t y)
{
    bool collision = false;

    // Sprites must wrap around if they are drawn completly off screen
    // TODO: Test this
    x %= Columns;
    y %= Lines;

    for (std::size_t i = 0; i < sprite.size() && y + i < Lines; ++i)
    {
        std::uint64_t line = buffer[y + i].load(std::memory_order_relaxed);
        const std::uint64_t mask = rotr(sprite[i], x + 8 - Columns);

        if (!collision && ((line ^ mask) != (line | mask)))
            collision = true;

        buffer[y + i].store(line ^ mask, std::memory_order_relaxed);
    }

    updated.store(true, std::memory_order_release);

    return collision;
}

void Frame::clear()
{
    for (auto& line : buffer)
        line.store(0, std::memory_order_relaxed);

    updated.store(true, std::memory_order_release);
}

void Frame::render(sf::RenderTarget& target, bool force = false)
{
    if (!updated.load(std::memory_order_acquire) && !force)
        return;

    sf::RectangleShape pixel({1.f, 1.f});
    pixel.setFillColor(sf::Color::White);

    target.clear(sf::Color::Black);

    for (std::size_t i = 0; i < Lines; ++i)
    {
        const float y = i;
        std::uint64_t line = buffer[i].load(std::memory_order_relaxed);

        for (std::size_t j = 0; j < Columns; ++j)
        {
            // unsigned long long is guaranteed to be at least 64 bits
            if (line & (1ull << j))
            {
                const float x = 63 - j;

                pixel.setPosition({x, y});
                target.draw(pixel);
            }
        }
    }

    updated.store(false, std::memory_order_relaxed);
}

void Frame::prepareTarget(sf::RenderTarget& target)
{
    /*
    * The render target keeps its own copy of the view object, so it is not
    * necessary to keep the original one alive after calling this function.
    *
    *  https://www.sfml-dev.org/documentation/2.5.1/classsf_1_1RenderTarget.php#a063db6dd0a14913504af30e50cb6d946
    */

    const sf::View view{sf::FloatRect{0.f, 0.f, Columns, Lines}};
    target.setView(view);
}
