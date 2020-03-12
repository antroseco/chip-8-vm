#include <ncurses.h>
#include <string>

struct ScreenGuard
{
    ScreenGuard();
    ~ScreenGuard();
};

class Window
{
    WINDOW* Handle;

public:
    Window(int Lines, int Columns);
    ~Window();

    Window(Window&) = delete;

    void Refresh() const;
    void WriteString(int y, int x, const std::string& String) const;
};
