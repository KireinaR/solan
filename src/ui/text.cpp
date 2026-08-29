#include "ui/text.h"
#include "ui/theme.h"
#include <cmath>
#include <cstdio>

std::string format_bytes(uint64_t bytes)
{
    constexpr uint64_t unit = 1000;
    char buf[32];

    if (bytes < unit)
    {
        std::snprintf(buf, sizeof(buf), "%llu B", static_cast<unsigned long long>(bytes));
        return std::string(buf);
    }

    const char* prefixes = "kMGTPE";
    uint64_t div = unit;
    int exp = 0;
    for (uint64_t n = bytes / unit; n >= unit; n /= unit)
    {
        div *= unit;
        exp++;
    }

    std::snprintf(buf, sizeof(buf), "%.1f %cB",
                  static_cast<double>(bytes) / static_cast<double>(div), prefixes[exp]);
    return std::string(buf);
}

std::string format_duration(double seconds)
{
    if (!(seconds > 0.0) || std::isinf(seconds) || std::isnan(seconds))
    {
        return std::string();
    }

    const long long total = static_cast<long long>(std::llround(seconds));
    if (total <= 0) return std::string();

    const long long hours = total / 3600;
    const long long minutes = (total % 3600) / 60;
    const long long secs = total % 60;

    char buf[48];
    if (hours > 0)
        std::snprintf(buf, sizeof(buf), "%lldh%02lldm%02llds", hours, minutes, secs);
    else if (minutes > 0)
        std::snprintf(buf, sizeof(buf), "%lldm%02llds", minutes, secs);
    else
        std::snprintf(buf, sizeof(buf), "%llds", secs);

    return std::string(buf);
}

std::string format_objects(size_t count)
{
    return std::to_string(count) + (count == 1 ? " object" : " objects");
}

namespace ui
{
    namespace
    {
        bool is_continuation(unsigned char b)
        {
            return (b & 0xC0) == 0x80;
        }
    }

    size_t visible_width(const std::string &text)
    {
        size_t width = 0;
        for (size_t i = 0; i < text.size(); ++i)
        {
            if (text[i] == '\033')
            {
                // Skip a CSI sequence: ESC [ ... final byte in @..~
                size_t j = i + 1;
                if (j < text.size() && text[j] == '[')
                {
                    ++j;
                    while (j < text.size() && !(text[j] >= '@' && text[j] <= '~')) ++j;
                }
                i = j;
                continue;
            }
            if (!is_continuation(static_cast<unsigned char>(text[i]))) ++width;
        }
        return width;
    }

    std::string clip(const std::string &text, size_t max_cols)
    {
        if (visible_width(text) <= max_cols) return text;

        std::string out;
        size_t width = 0;
        for (size_t i = 0; i < text.size(); ++i)
        {
            if (text[i] == '\033')
            {
                size_t j = i + 1;
                if (j < text.size() && text[j] == '[')
                {
                    ++j;
                    while (j < text.size() && !(text[j] >= '@' && text[j] <= '~')) ++j;
                }
                out.append(text, i, j - i + 1);
                i = j;
                continue;
            }
            if (!is_continuation(static_cast<unsigned char>(text[i])))
            {
                if (width >= max_cols) break;
                ++width;
            }
            out += text[i];
        }
        out += col::RESET;
        return out;
    }

    std::string truncate_left(const std::string &plain, size_t max_cols)
    {
        const size_t width = visible_width(plain);
        if (width <= max_cols) return plain;
        if (max_cols <= 1) return "…";

        const size_t drop = width - (max_cols - 1);
        size_t i = 0;
        size_t seen = 0;
        while (i < plain.size() && seen < drop)
        {
            ++i;
            while (i < plain.size() && is_continuation(static_cast<unsigned char>(plain[i]))) ++i;
            ++seen;
        }
        return "…" + plain.substr(i);
    }

    std::string pad_right(const std::string &plain, size_t cols)
    {
        const size_t width = visible_width(plain);
        if (width >= cols) return plain;
        return plain + std::string(cols - width, ' ');
    }
}
