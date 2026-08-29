#include "ui/theme.h"
#include <algorithm>
#include <cctype>
#include <cmath>

namespace col
{
    namespace
    {
        Rgb mix(Rgb a, Rgb b, double t)
        {
            return Rgb{
                static_cast<int>(std::lround(a.r + (b.r - a.r) * t)),
                static_cast<int>(std::lround(a.g + (b.g - a.g) * t)),
                static_cast<int>(std::lround(a.b + (b.b - a.b) * t))};
        }
    }

    std::string fg(Rgb c)
    {
        return "\033[38;2;" + std::to_string(c.r) + ";" +
               std::to_string(c.g) + ";" + std::to_string(c.b) + "m";
    }

    std::string ramp(double t)
    {
        t = std::min(1.0, std::max(0.0, t));

        if (t < 0.5)
            return fg(mix(ACCENT_DEEP_RGB, ACCENT_RGB, t * 2.0));

        return fg(mix(ACCENT_RGB, ACCENT_SOFT_RGB, (t - 0.5) * 2.0));
    }
}

namespace ui
{
    namespace
    {
        std::string wrap(const char *code, const std::string &text)
        {
            return std::string(code) + text + col::RESET;
        }
    }

    std::string help(const std::string &text)    { return wrap(col::MUTED, text); }
    std::string faint(const std::string &text)   { return wrap(col::FAINT, text); }
    std::string element(const std::string &text) { return wrap(col::ACCENT, text); }
    std::string bold(const std::string &text)    { return wrap(col::BOLD, text); }
    std::string danger(const std::string &text)  { return wrap(col::DANGER, text); }
    std::string warning(const std::string &text) { return wrap(col::WARNING, text); }
    std::string success(const std::string &text) { return wrap(col::SUCCESS, text); }

    std::string label(const std::string &text)
    {
        std::string upper;
        upper.reserve(text.size());
        for (char c : text)
        {
            upper += static_cast<char>(std::toupper(static_cast<unsigned char>(c)));
        }
        return wrap(col::MUTED, upper);
    }

    std::string spaced(const std::string &text)
    {
        std::string out;
        for (size_t i = 0; i < text.size(); ++i)
        {
            if (i) out += ' ';
            out += static_cast<char>(std::toupper(static_cast<unsigned char>(text[i])));
        }
        return out;
    }
}
