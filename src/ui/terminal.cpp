#include "ui/terminal.h"

#ifdef _WIN32
#include <windows.h>

void enable_ansi_support()
{
    SetConsoleOutputCP(CP_UTF8);

    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hOut == INVALID_HANDLE_VALUE) return;

    DWORD mode = 0;
    if (!GetConsoleMode(hOut, &mode)) return;

    mode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
    SetConsoleMode(hOut, mode);
}

int terminal_width()
{
    CONSOLE_SCREEN_BUFFER_INFO info;
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hOut != INVALID_HANDLE_VALUE && GetConsoleScreenBufferInfo(hOut, &info))
    {
        const int width = info.srWindow.Right - info.srWindow.Left + 1;
        if (width > 0) return width;
    }
    return 80;
}

bool stdout_is_tty()
{
    DWORD mode = 0;
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    return hOut != INVALID_HANDLE_VALUE && GetConsoleMode(hOut, &mode) != 0;
}
#else
#include <sys/ioctl.h>
#include <unistd.h>

void enable_ansi_support() {}

int terminal_width()
{
    struct winsize ws;
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == 0 && ws.ws_col > 0)
    {
        return static_cast<int>(ws.ws_col);
    }
    return 80;
}

bool stdout_is_tty()
{
    return isatty(STDOUT_FILENO) != 0;
}
#endif
