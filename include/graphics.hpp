#pragma once

#include "utility.hpp"

#include <array>
#include <atomic>
#include <cstddef>
#include <cstdint>

namespace sf
{
class RenderTarget;
}

class Frame
{
public:
    static constexpr std::size_t Lines = 0x20;
    static constexpr std::size_t Columns = 0x40;

    // Draws a sprite, with each byte on a separate line
    [[nodiscard]] bool drawSprite(byte_view sprite, std::size_t x, std::size_t y);
    void clear();
    void render(sf::RenderTarget& target, bool force);

    static void prepareTarget(sf::RenderTarget& target);

private:
    std::array<std::atomic_uint64_t, Lines> buffer = {};
    std::atomic_bool updated = true;
};
