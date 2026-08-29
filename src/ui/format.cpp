#include "format.h"
#include <cstdio>

std::string format_bytes(uint64_t bytes)
{
    const char* units[] = {"B", "KB", "MB", "GB", "TB"};
    double size = static_cast<double>(bytes);
    int unit = 0;
    while (size >= 1024.0 && unit < 4)
    {
        size /= 1024.0;
        unit++;
    }

    char buf[32];
    if (unit == 0)
        std::snprintf(buf, sizeof(buf), "%llu %s", static_cast<unsigned long long>(bytes), units[unit]);
    else
        std::snprintf(buf, sizeof(buf), "%.1f %s", size, units[unit]);

    return std::string(buf);
}