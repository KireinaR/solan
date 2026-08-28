#include "progress.h"
#include "colors.h"
#include "../net/protocol.h"
#include <iostream>
#include <iomanip>
#include <string>

void print_progress(uint64_t current, uint64_t total, const char* label)
{
    double ratio = total > 0 ? static_cast<double>(current) / static_cast<double>(total) : 0.0;
    int filled = static_cast<int>(ratio * PROGRESS_BAR_WIDTH);
    int empty = PROGRESS_BAR_WIDTH - filled;

    std::cout << "\r" << label << " ["
               << col::GREEN << std::string(filled, '=') << col::RESET
               << std::string(empty, ' ')
               << "] " << std::fixed << std::setprecision(1) << (ratio * 100.0) << "%   ";
    std::cout.flush();
}

void finish_progress()
{
    std::cout << std::endl;
}