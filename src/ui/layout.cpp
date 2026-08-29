#include "ui/layout.h"
#include "ui/terminal.h"
#include "ui/text.h"
#include "ui/theme.h"
#include <algorithm>
#include <iostream>

namespace ui
{
    const std::string PAD(MARGIN, ' ');

    int content_width()
    {
        const int available = terminal_width() - 2 * MARGIN;
        return std::max(24, std::min(MAX_WIDTH, available));
    }

    std::string rule(int width)
    {
        if (width <= 0) width = content_width();

        std::string bar;
        bar.reserve(static_cast<size_t>(width) * 3);
        for (int i = 0; i < width; ++i) bar += "─";
        return faint(bar);
    }

    void separator()
    {
        std::cout << PAD << rule() << "\n";
    }

    void line(const std::string &text)
    {
        std::cout << PAD << text << "\n";
    }

    // Label left, value right, the way the site sets its metadata rows.
    void row(const std::string &left, const std::string &right, int width)
    {
        if (width <= 0) width = content_width();

        const size_t used = visible_width(left) + visible_width(right);
        const std::string gap(used < static_cast<size_t>(width) ? width - used : 1, ' ');
        std::cout << PAD << left << gap << right << "\n";
    }

    void blank()
    {
        std::cout << "\n";
    }

    void task(const std::string &text)
    {
        std::cout << PAD << text << "\n";
    }

    void task_ok(const std::string &text)
    {
        std::cout << PAD << success(text) << "\n";
    }

    void task_fail(const std::string &text)
    {
        std::cout << PAD << danger(text) << "\n";
    }
}
