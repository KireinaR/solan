#include "ui/progress.h"
#include "ui/theme.h"
#include <algorithm>
#include <cmath>
#include <cstdio>

std::string progress_bar(double ratio, int total_width)
{
    ratio = std::min(1.0, std::max(0.0, ratio));

    char percent[16];
    std::snprintf(percent, sizeof(percent), "  %.2f%%", ratio * 100.0);

    int bar_width = total_width - static_cast<int>(std::string(percent).size());
    if (bar_width < 8) bar_width = 8;

    int filled = static_cast<int>(std::lround(ratio * bar_width));
    filled = std::min(bar_width, std::max(0, filled));

    std::string out;
    out.reserve(static_cast<size_t>(bar_width) * 24);

    for (int i = 0; i < filled; ++i)
    {
        const double t = bar_width > 1 ? static_cast<double>(i) / (bar_width - 1) : 0.0;
        out += col::ramp(t);
        out += "█";
    }

    if (filled < bar_width)
    {
        out += col::FAINT;
        for (int i = filled; i < bar_width; ++i) out += "░";
    }

    out += col::MUTED;
    out += percent;
    out += col::RESET;

    return out;
}
