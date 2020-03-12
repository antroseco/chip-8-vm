#include "../include/window.hpp"

#include <system_error>

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
}

ScreenGuard::~ScreenGuard()
{
    endwin();
}

Window::Window(int Lines, int Columns) : Handle(newwin(Lines + 2, Columns + 2, 0, 0))
{
    //whline(Handle, ACS_CKBOARD, Columns);
    wborder(Handle, 0, 0, 0, 0, 0, 0, 0, 0);
}

Window::~Window()
{
    delwin(Handle);
}

void Window::Refresh() const
{
    int err = wrefresh(Handle);

    if (err != 0)
        throw std::system_error(err, std::system_category(), "wrefresh");
}

void Window::WriteString(int y, int x, const std::string& String) const
{
    int err = mvwaddstr(Handle, y + 1, x + 1, String.c_str());

    if (err != 0)
        throw std::system_error(err, std::system_category(), "mvwaddstr");
}
