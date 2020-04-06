#pragma once

#include <SFML/Graphics.hpp>

#include <array>
#include <atomic>
#include <mutex>
#include <vector>

class Frame
{
public:
    static constexpr std::size_t Lines = 0x20;
    static constexpr std::size_t Columns = 0x40;

    // Draws a sprite, with each byte on a separate line
    [[nodiscard]] bool drawSprite(const std::vector<std::uint8_t>& sprite, std::size_t x, std::size_t y) noexcept;
    void clear() noexcept;
    void render(sf::RenderTarget& target, bool force);

    static void prepareTarget(sf::RenderTarget& target);

private:
    std::array<std::uint64_t, Lines> buffer{0};
    std::atomic_bool updated = true;
    std::mutex buffer_mutex;
};
