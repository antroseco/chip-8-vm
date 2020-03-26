#pragma once

#include <array>
#include <bitset>
#include <ncurses.h>
#include <string>
#include <vector>

struct ScreenGuard
{
    ScreenGuard();
    ~ScreenGuard();
};

class Window
{
public:
    static constexpr std::size_t Lines = 32;
    static constexpr std::size_t Columns = 64;

private:
    WINDOW* const Handle;
    std::array<std::bitset<Columns>, Lines> Data;

    void DrawLine(std::size_t Line) const;

public:
    Window(int x0, int y0);
    ~Window();

    Window(Window&) = delete;

    // The screen is updated only when Refresh is called
    void Refresh() const;
    // Clear the window
    void Clear();
    void WriteString(int y, int x, const std::string& String) const;
    // Draws a sprite, with each byte on a separate line
    bool DrawSprite(const std::vector<std::uint8_t>& Sprite, std::size_t x, std::size_t y);
    char GetKey() const;
};
