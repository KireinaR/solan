#pragma once
#include <cstdint>

void print_progress(uint64_t current, uint64_t total, const char* label);
void finish_progress();