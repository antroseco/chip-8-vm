#include "window.hpp"

#include <iterator>
#include <system_error>

auto ror(std::bitset<Window::Columns> x, std::size_t Count)
{
    return (x >> Count) | (x << (-Count & (x.size() * 8 - 1)));
}

/*
                The screen (stdscr)
(0,0)*----------------------------------* (0, COLUMNS-1)
     |                                  |
     |                                  |
     |    (y0,x0)                       |
     |      ---------------             |
     |      |             |             |
     |      |             |             |
     |      |     win     |nlines       |
     |      |             |             |
     |      |             |             |
     |      |             |             |
     |      ---------------             |
     |          ncols                   |
     |                                  |
     *----------------------------------*
(LINES-1, 0)                             (LINES-1, COLUMNS-1)
*/

ScreenGuard::ScreenGuard()
{
    initscr();                // Required
    cbreak();                 // Disable line buffering
    noecho();                 // Disables echoing of typed characters
    curs_set(0);              // Disables the cursor
    intrflush(stdscr, false); // Disables buffer flusing on keyboard interrupts
    keypad(stdscr, true);     // Interpret function keys
    setlocale(LC_ALL, "");    // Set locale
}

ScreenGuard::~ScreenGuard()
{
    endwin();
}

Window::Window(int x0, int y0) : Handle(newwin(Lines + 2, Columns * 2 + 2, y0, x0))
{
    nodelay(Handle, true); // Make getch a non-blocking call
    wborder(Handle, 0, 0, 0, 0, 0, 0, 0, 0);
}

Window::~Window()
{
    delwin(Handle);
}

void Window::Refresh() const
{
    wrefresh(Handle);
}

void Window::WriteString(int y, int x, const std::string& String) const
{
    mvwaddstr(Handle, y + 1, x + 1, String.c_str());
}

bool Window::DrawSprite(const std::vector<std::uint8_t>& Sprite, std::size_t x, std::size_t y)
{
    bool ErasedPixel = false;

    for (std::size_t i = 0; i < Sprite.size(); ++i)
    {
        // Check if we are still drawing inside the screen
        if (y + i >= Lines)
            break;

        std::bitset<Columns>& Line = Data[y + i];
        const std::bitset<Columns> Mask = ror(Sprite[i], x + 8 - Columns);

        if (!ErasedPixel && ((Line ^ Mask) != (Line | Mask)))
            ErasedPixel = true;

        Line ^= Mask;

        DrawLine(y + i);
    }

    return ErasedPixel;
}

void Window::DrawLine(std::size_t Line) const
{
    const auto& Bits = Data[Line];

    wmove(Handle, Line + 1, 1);

    for (std::size_t i = Bits.size() - 1; i != (std::size_t)-1; --i)
    {
        waddch(Handle, Bits.test(i) ? ACS_CKBOARD : ' ');
        waddch(Handle, Bits.test(i) ? ACS_CKBOARD : ' ');
    }
}

void Window::Clear()
{
    for (auto& i : Data)
        i.reset();

    wclear(Handle);
    wborder(Handle, 0, 0, 0, 0, 0, 0, 0, 0);
}

char Window::GetKey() const
{
    const int input = wgetch(Handle);

    return input == ERR ? 0 : input;
}
