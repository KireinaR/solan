#include "spinner.h"
#include "colors.h"
#include <iostream>
#include <thread>
#include <chrono>

void run_spinner(const std::string &label, std::atomic<bool> &stop_flag)
{
    static const char* frames[] = {"⣾", "⣽", "⣻", "⢿", "⡿", "⣟", "⣯", "⣷"};
    constexpr int frame_count = 8;
    int i = 0;

    while (!stop_flag.load())
    {
        std::cout << "\r" << col::CYAN << frames[i % frame_count] << col::RESET << "  " << label << std::flush;
        i++;
        std::this_thread::sleep_for(std::chrono::milliseconds(80));
    }

    std::cout << "\r" << std::string(label.size() + 4, ' ') << "\r" << std::flush;
}