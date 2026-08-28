#include "progress.h"
#include "../net/protocol.h"
#include <iostream>
#include <iomanip>

void print_progress(uint64_t current, uint64_t total, const char* label)
{
    double ratio = total > 0 ? static_cast<double>(current) / static_cast<double>(total) : 0.0;
    int filled = static_cast<int>(ratio * PROGRESS_BAR_WIDTH);

    std::cout << "\r" << label << " [";
    for (int i = 0; i < PROGRESS_BAR_WIDTH; ++i)
    {
        if (i < filled)
            std::cout << "\u2588"; // full block
        else if (i == filled)
            std::cout << "\u2592"; // partial/leading edge shade
        else
            std::cout << "\u2591"; // light shade (empty)
    }
    std::cout << "] " << std::fixed << std::setprecision(1) << (ratio * 100.0) << "%";
    std::cout.flush();
}

void finish_progress()
{
    std::cout << std::endl;
}