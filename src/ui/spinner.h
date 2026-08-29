#pragma once
#include <atomic>
#include <string>

void run_spinner(const std::string &label, std::atomic<bool> &stop_flag);